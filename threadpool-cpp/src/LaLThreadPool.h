#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>

class LaLThreadPool {
 public:
  explicit LaLThreadPool(int threadSize = (int)std::thread::hardware_concurrency()) : is_done(false) {
    for (size_t i = 0; i < threadSize; ++i) {
      m_threads.emplace_back([this] {
        while (true) {
          std::function<void()> task;
          {
            WRITE_LOCK lock(m_queueMutex);
            m_condition.wait(lock, [this] { return is_done || !m_tasks.empty(); });
            if (is_done && m_tasks.empty()) {
              return;
            }
            task = std::move(m_tasks.front());
            m_tasks.pop();
          }
          task();
        }
      });
    }
  }
  template<class F, class... Args>
  void enqueue(F&& f, Args&&... args) {
    {
      WRITE_LOCK lock(m_queueMutex);
      m_tasks.emplace([=] {
#if __cplusplus >= 201411L
        std::invoke(f, args...);
#elif __cplusplus >= 202106L
        std::invoke_r(f, args...);
#else
        std::__invoke(f, args...);
#endif
      });
    }
    m_condition.notify_one();
  }

 private:
  std::vector<std::thread> m_threads;
  std::queue<std::function<void()>> m_tasks;
  using READ_LOCK = std::lock_guard<std::mutex>;
  using WRITE_LOCK = std::unique_lock<std::mutex>;
  std::mutex m_queueMutex;
  std::condition_variable m_condition;
  bool is_done;
};