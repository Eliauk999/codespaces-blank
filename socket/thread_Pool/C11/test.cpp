#include <iostream>
#include <vector>
#include <chrono>
#include <future>
#include "ThreadPool.h"

int foo()
{
    std::cout << "Starting foo()" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Finished foo()" << std::endl;

    return 42;
}

int main()
{

    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    // for (int i = 0; i < 8; ++i)
    // {
    //     results.emplace_back(
    //         pool.enqueue([i]
    //                      {
    //         std::cout << "hello " << i << std::endl;
    //         std::this_thread::sleep_for(std::chrono::seconds(1));
    //         std::cout << "world " << i << std::endl;
    //         return i*i; }));
    // }

    // std::vector<std::future<int>> results;
    for (int i = 0; i < 8; ++i)
    {
        results.emplace_back(std::async(foo));
    }

    for (auto &result : results)
    {
        int value = result.get();
        std::cout << "foo() returned " << value << std::endl;
    }
    // for (auto &&result : results) // 通过future.get()获取返回值
    //     std::cout << result.get() << ' ';
    // std::cout << std::endl;

    return 0;
}
