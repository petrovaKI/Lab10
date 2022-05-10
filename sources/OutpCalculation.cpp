// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>

#include "OutpCalculation.hpp"
#include <iostream>

My_BD::My_BD(std::string& input_dir,
                               std::string& output_dir,
                               size_t number_of_threads)
    : ProdQueue_(),
      ConsQueue_(),
      input_(input_dir),
      output_(output_dir),
      HashPool_(number_of_threads) {
  //STATUS-
  // Значения этого типа возвращаются большинством функций
  // в RocksDB, которые могут столкнуться с ошибкой
  rocksdb::Status s{};
  std::vector<std::string> names;
  //ColumnFamilyDescriptor - структура с именем семейства столбцов
  // и ColumnFamilyOptions
  std::vector<rocksdb::ColumnFamilyDescriptor> desc;
  try {
    //List Column Families  - это статическая функция,
    // которая возвращает список всех семейств столбцов,
    // присутствующих в данный момент в базе данных.
    s = rocksdb::DB::ListColumnFamilies(rocksdb::DBOptions(), input_, &names);
    if (!s.ok()) throw std::runtime_error("ListColumnFamilies is failed");

    //выделяем место в векторе под names
    desc.reserve(names.size());
    // заполняем вектор семействами столбцов
    for (auto& x : names) {
      desc.emplace_back(x, rocksdb::ColumnFamilyOptions());
    }
    //OpenForReadOnly -
    //Поведение аналогично DB::Open, за исключением того,
    // что он открывает БД в режиме только для чтения.
    // Одна большая разница заключается в том, что при открытии БД
    // только для чтения не нужно указывать все семейства столбцов -
    // можно открыть только подмножество семейств столбцов.
    s = rocksdb::DB::OpenForReadOnly(rocksdb::DBOptions(), input_, desc,
                                     &fromHandles_, &inpBD_);
    if (!s.ok())
      throw std::runtime_error("OpenForReadOnly of input DB is failed");
  //очищаем вектор с именами (строки)
    names.erase(names.begin());
//------создаём новую БД для хешей--------------------
    //Options структура определяет, как RocksDB ведет себя
    rocksdb::Options options;
    //проверяем создана ли БД
    options.create_if_missing = true;
   //открываем БД на запись
    s = rocksdb::DB::Open(options, output_, &outputBD_);
    if (!s.ok()) throw std::runtime_error("Open of output DB is failed");
  //создаём семецства столбцов
    //CreateColumnFamilies - создает семейство столбцов,
    //указанное с параметром и именем names, и возвращает
    // ColumnFamilyHandle через аргумент outHandles_.
    outputBD_->CreateColumnFamilies(rocksdb::ColumnFamilyOptions(), names,
                                    &outHandles_);

    outHandles_.insert(outHandles_.begin(), outputBD_->DefaultColumnFamily());
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
  }
//------------------------------------------------------------
}

//парсим исходные данные в первоначальной БД
void My_BD::parse_inp_BD() {
  std::vector<rocksdb::Iterator*> iterators;
  rocksdb::Iterator* it;

  for (size_t i = 0; i < fromHandles_.size(); ++i) {
    it = inpBD_->NewIterator(rocksdb::ReadOptions(), fromHandles_[i]);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      //ставим в очередь пары ключ-значение
      ProdQueue_.Push({i, it->key().ToString(), it->value().ToString()});
    }
    iterators.emplace_back(it);
    it = nullptr;
  }

  for (auto& x : iterators) {
    delete x;
  }

  ParseFlag_ = true;
}

//деструктор
My_BD::~My_BD() {
  try {
    //Перед удалением базы данных нужно закрыть
    //все семейства столбцов,
    //вызвав DestroyColumnFamilyHandle() со всеми дескрипторами.
    rocksdb::Status s;
    if (!fromHandles_.empty() && inpBD_ != nullptr) {
      for (auto& x : fromHandles_) {
        s = inpBD_->DestroyColumnFamilyHandle(x);
        if (!s.ok()) {
          throw std::runtime_error("Destroy From Handle failed in destructor");
        }
      }
      fromHandles_.clear();
      s = inpBD_->Close();
      if (!s.ok()) {
        throw std::runtime_error("Closing of fromDB in destructor");
      }
      delete inpBD_;
    }

    if (!outHandles_.empty() && outputBD_ != nullptr) {
      for (auto& x : outHandles_) {
        s = outputBD_->DestroyColumnFamilyHandle(x);
        if (!s.ok()) {
          throw std::runtime_error(
              "Destroy Output Handle failed in destructor");
        }
      }
      outHandles_.clear();
    }
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
  }
}
//запись хешированных данных в новую БД
void My_BD::write_val_to_BD(Entry&& Key_Hash) {
  try {
    rocksdb::Status s = outputBD_->Put(rocksdb::WriteOptions(),
                                       outHandles_[Key_Hash.Handle],
                                       Key_Hash.Key, Key_Hash.Value);
    BOOST_LOG_TRIVIAL(info)
        <<"[" << Key_Hash.Key << "] " << " [" << Key_Hash.Value << "] "
        << " [-NEW DATA BASE-]";
    if (!s.ok()) {
      throw std::runtime_error("Writing in output DB is failed");
    }
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
  }
}
//-----------вычисляем хеш--------------------------------------------
std::string calc_hash(const std::string& key, const std::string& value) {
  return picosha2::hash256_hex_string(std::string(key + value));
}
//----------ставим в очередь на запись строку для новой БД--------------
void My_BD::make_cons_queue(Entry& en) {
  ConsQueue_.Push({en.Handle, en.Key, calc_hash(en.Key, en.Value)});
}
//--------------------------------------------------------------------
//задаём пул потоков
void My_BD::make_cons_pool() {
  Entry item;
  while (!ParseFlag_ || !ProdQueue_.Empty()) {
    if (ProdQueue_.Pop(item)) {
      HashPool_.enqueue([this](Entry x) { make_cons_queue(x); }, item);
    }
  }
  HashFlag_ = true;
}
//------------------------------------------------------------------
//------записываем всю очередь в новую БД---------------------------
void My_BD::write_new_BD() {
  Entry item;
  while (!ConsQueue_.Empty() || !HashFlag_) {
    if (ConsQueue_.Pop(item)) {
      write_val_to_BD(std::move(item));
    }
  }
  WriteFlag_ = true;
}
//-------запускаем потоки и выполняем парсинг и запись---------------
void My_BD::start_process() {
  std::thread producer([this]() { parse_inp_BD(); });

  std::thread consumer([this]() { write_new_BD(); });

  producer.join();
  make_cons_pool();
  consumer.join();

  while (!HashFlag_ || !ParseFlag_ || !WriteFlag_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}
