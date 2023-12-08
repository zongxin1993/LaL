
#include <iostream>

#include "../src/LaLThreadPool.h"

void taskFunction(int id) {
  std::cout << "Task " << id << " is running.\n" << std::endl;
}

int main() {
  LaLThreadPool pool(4); // 创建一个拥有4个线程的线程池

  for (int i = 0; i < 8; ++i) {
    pool.enqueue(taskFunction, i);
  }

  std::this_thread::sleep_for(std::chrono::seconds(2));

  return 0;
}