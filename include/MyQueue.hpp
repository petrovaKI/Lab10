// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>

#ifndef INCLUDE_MYQUEUE_HPP_
#define INCLUDE_MYQUEUE_HPP_

#include <mutex>
#include <queue>
#include <utility>
#include <vector>

template <typename T>
class MyQueue {
 public:
  MyQueue() : queue_(), mutex_(){}
  void Push(T& other) {
    std::lock_guard lockGuard{mutex_};
    queue_.push(other);
  }
  void Push(T&& other) {
    std::lock_guard lockGuard{mutex_};
    queue_.push(std::move(other));
  }
  void Push(std::vector<T>&& others) {
    std::lock_guard lockGuard{mutex_};
    for (auto& x : others) queue_.push(x);
  }
  bool Empty() { return queue_.empty(); }
  bool Pop(T& item) {
    if (queue_.empty()) return false;

    mutex_.lock();
    item = std::move(queue_.front());
    queue_.pop();
    mutex_.unlock();
    return true;
  }

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
};

#endif  // INCLUDE_MYQUEUE_HPP_
