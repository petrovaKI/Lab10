// Copyright 2022 Petrova Kseniya <petrovaKI>

#ifndef INCLUDE_INPUTBD_HPP_
#define INCLUDE_INPUTBD_HPP_

#include <string>
#include <rocksdb/db.h>
#include <boost/log/trivial.hpp>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/advanced_options.h>
#include <picosha2.h>

//создаём первоначальную БД
void make_inp_BD(const std::string& directory);


#endif // INCLUDE_INPUTBD_HPP_
