//
// Created by pe on 2020/7/17.
//

#ifndef TWOCAMERA_BLOCKINGQUEUE_H
#define TWOCAMERA_BLOCKINGQUEUE_H

#include <condition_variable>
#include <deque>
#include <mutex>

template <typename T> class BlockingQueue {
private:
  std::mutex d_mutex;
  std::condition_variable d_condition;
  std::deque<T> d_queue;
  int caps;

public:
  BlockingQueue(int cap = INT32_MAX) { caps = cap; }
  void push(T const &value) {
    {
      std::unique_lock<std::mutex> lock(this->d_mutex);
      this->d_condition.wait(
          lock, [this]() { return this->d_queue.size() < this->caps; });
      d_queue.push_front(value);
    }
    this->d_condition.notify_one();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    this->d_condition.wait(lock, [=] { return !this->d_queue.empty(); });
    T rc(std::move(this->d_queue.back()));
    this->d_queue.pop_back();
    this->d_condition.notify_one();
    return rc;
  }

  bool tryPop(T& result, int timeoutMs = 100) {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    if (this->d_condition.wait_for(lock, std::chrono::milliseconds(timeoutMs), 
                                   [this] { return !this->d_queue.empty(); })) {
      result = std::move(this->d_queue.back());
      this->d_queue.pop_back();
      this->d_condition.notify_one();
      return true;
    }
    return false;
  }

  bool isEmpty() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    return this->d_queue.empty();
  }

  void clear() {
    {
      std::unique_lock<std::mutex> lock(this->d_mutex);
      d_queue.clear();
    }
    this->d_condition.notify_one();
  }

  int size() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    return d_queue.size();
  }
};

#endif // TWOCAMERA_BLOCKINGQUEUE_H
