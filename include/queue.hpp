// Copyright 2022 Petrova Kseniya <petrovaKI>

#ifndef INCLUDE_MYQUEUE_HPP_
#define INCLUDE_MYQUEUE_HPP_

#include <mutex>
#include <queue>
#include <utility>
#include <vector>

template <typename T>
class Queue {
 public:
  void push(T&& other) {
    std::lock_guard lockGuard{mtx};
    queue_.push(std::move(other));
  }
  bool empty() { return queue_.empty(); }
  bool pop(T& item) {
    if (queue_.empty()) return false;

    mtx.lock();
    item = std::move(queue_.front());
    queue_.pop();
    mtx.unlock();
    return true;
  }

 private:
  std::mutex mtx;
  std::queue<T> queue_;
};

#endif  // INCLUDE_MYQUEUE_HPP_
