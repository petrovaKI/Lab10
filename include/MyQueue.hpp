// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>

#ifndef INCLUDE_MYQUEUE_HPP_
#define INCLUDE_MYQUEUE_HPP_

#include <mutex>
#include <queue>
#include <utility>
#include <vector>

template <typename T>
class Queue {
 public:
  void Push(T&& other) {
    std::lock_guard lockGuard{mutex};
    queue_.push(std::move(other));
  }
  bool Empty() { return queue_.empty(); }
  bool Pop(T& item) {
    if (queue_.empty()) return false;

    mutex.lock();
    item = std::move(queue_.front());
    queue_.pop();
    mutex.unlock();
    return true;
  }

 private:
  std::mutex mutex;
  std::queue<T> queue_;
};

#endif  // INCLUDE_MYQUEUE_HPP_
