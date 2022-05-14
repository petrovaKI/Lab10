// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>

#include <InputBD.hpp>
#include <iostream>

// СОЗДАНИЕ БД (принимает путь до файла)
void make_inp_BD(const std::string& directory) {
  const unsigned int NUMBER_OF_COLUMNS = 3;
  const unsigned int NUMBER_OF_VALUES = 5;

  try {
    //  СОЗДАЁМ И ОТКРЫВАЕМ БАЗУ ДАННЫХ
    // переменная для опций
    rocksdb::Options options;
    // "создать БД, если она не существует"
    options.create_if_missing = true;
    // db - private член класса -
    // переменная для БД
    rocksdb::DB* db = nullptr;
    // открываем/создаем БД
    rocksdb::Status status = rocksdb::DB::Open(options, directory, &db);

    if (!status.ok()) throw std::runtime_error{"DB::Open failed"};

    //    ЗАДАЁМ СЕМЕЙСТВА СТОЛБЦОВ (ColumnFamilies) в БД
    std::vector<std::string> column_family;

    // задаём размер вектора
    column_family.reserve(NUMBER_OF_COLUMNS);
    //заполняем вектор семействами столбцов
    for (unsigned int i = 0; i < NUMBER_OF_COLUMNS; ++i) {
      column_family.emplace_back("ColumnFamily_" + std::to_string(i + 1));
    }

    std::vector<rocksdb::ColumnFamilyHandle*> handles;
    status = db->CreateColumnFamilies(rocksdb::ColumnFamilyOptions(),
                                      column_family, &handles);
    if (!status.ok()) throw std::runtime_error{"CreateColumnFamilies failed"};

    //   ЗАПОЛЯЕМ БД СЛУЧАЙНЫМИ ЗНАЧЕНИЯМИ
    std::string key;
    std::string value;
    for (unsigned int i = 0; i < NUMBER_OF_COLUMNS; ++i) {
      for (unsigned int j = 0; j < NUMBER_OF_VALUES; ++j) {
        key = "key-" + std::to_string((i * NUMBER_OF_VALUES) + j);
        value = "value-" + std::to_string(std::rand() % 100);
        status = db->Put(rocksdb::WriteOptions(), handles[i],
                         rocksdb::Slice(key), rocksdb::Slice(value));
        if (!status.ok())
          throw std::runtime_error{"Putting [" + std::to_string(i + 1) + "][" +
                                   std::to_string(j) + "] failed"};

        BOOST_LOG_TRIVIAL(info) << "Added [" << key << "]:[" << value
                                 << "] -- [" << i + 1 << " family column ]"
                                 <<" -- [ FIRST DATA BASE ]";
      }
    }
    //Перед удалением базы данных нужно закрыть
    //все семейства столбцов,
    //вызвав DestroyColumnFamilyHandle() со всеми дескрипторами.
    // закрываем БД
    for (auto& x : handles) {
      status = db->DestroyColumnFamilyHandle(x);
      if (!status.ok()) throw std::runtime_error{"DestroyColumnFamily failed"};
    }

    delete db;
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
  }
}
