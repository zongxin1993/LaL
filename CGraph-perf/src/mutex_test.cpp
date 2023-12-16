#include <iostream>
#include <thread>
#include <mutex>

const int NUM_PHILOSOPHERS = 5;

std::mutex forks[NUM_PHILOSOPHERS];

void philosopher(int id)
{
    int left_fork = id;
    int right_fork = (id + 1) % NUM_PHILOSOPHERS;

    while (true)
    {
        // 模拟思考
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 拿起左边的餐叉
        forks[left_fork].lock();
        std::cout << "Philosopher " << id << " picks up left fork" << std::endl;

        // 拿起右边的餐叉
        forks[right_fork].lock();
        std::cout << "Philosopher " << id << " picks up right fork" << std::endl;

        // 吃饭
        std::cout << "Philosopher " << id << " is eating" << std::endl;

        // 放下右边的餐叉
        forks[right_fork].unlock();
        std::cout << "Philosopher " << id << " puts down right fork" << std::endl;

        // 放下左边的餐叉
        forks[left_fork].unlock();
        std::cout << "Philosopher " << id << " puts down left fork" << std::endl;
    }
}

int main()
{
    std::thread philosophers[NUM_PHILOSOPHERS];

    // 创建哲学家线程
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i)
    {
        philosophers[i] = std::thread(philosopher, i);
    }

    // 等待线程结束
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i)
    {
        philosophers[i].join();
    }

    return 0;
}