// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>

#ifndef  INCLUDE_OUTPCALCULATION_HPP_
#define  INCLUDE_OUTPCALCULATION_HPP_

#include <rocksdb/db.h>

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <boost/log/trivial.hpp>

#include "ThreadPool.hpp"
#include "MyQueue.hpp"
#include "InputBD.hpp"

std::string calc_hash(const std::string& key, const std::string& value);

//запись таблицы (БД)
struct Entry{
  size_t Handle; //дескриптор
  std::string Key; //ключ
  std::string Value; //значение
};


class My_BD {
 public:
  My_BD(std::string& input_filename, std::string& output_filename,
                 size_t number_of_threads);
  ~My_BD();
  void write_val_to_BD(Entry&& KeyHash);
  void parse_inp_BD();
  void make_cons_queue(Entry& en);
  void write_new_BD();
  void start_process();
  void make_cons_pool();

 private:
  bool ParseFlag_ = false;
  bool HashFlag_ = false;
  bool WriteFlag_ = false;

  MyQueue<Entry> ProdQueue_;
  MyQueue<Entry> ConsQueue_;
  std::string input_; //файл с начальной БД
  std::string output_; //файл с конечной БД
  //Семейства столбцов обрабатываются и ссылаются
  // с помощью  ColumnFamilyHandle
  std::vector<rocksdb::ColumnFamilyHandle*> fromHandles_;
  std::vector<rocksdb::ColumnFamilyHandle*> outHandles_;
  //начальная и конечная БД
  rocksdb::DB* inpBD_ = nullptr;
  rocksdb::DB* outputBD_ = nullptr;
  //пул потоков
  ThreadPool HashPool_;
};

#endif  //  INCLUDE_OUTPCALCULATION_HPP_
