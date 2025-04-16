#include<iostream>
#include<thread>
#include<mutex>

#include "ConnectionPool.h"
#include "Connection.h"

using namespace std;

/* 简单的框架实现 */

void userRequest(int arr[])  // 用户的线程函数
{
    auto time_start = std::chrono::steady_clock::now();
    // 如果进程超过10s没有执行相关操作，则回收连接，有则刷新计时
    
    // 从连接池中获取一个连接
    cout << "获取连接" << endl;
    Connection* conn = ConnectionPool::getInstance()->takeOutFromPool();

    int label = 1;
    int index = 0;

    while (label)
    {
        cout << std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::steady_clock::now() - time_start).count() << endl;
        if (std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::steady_clock::now() - time_start).count() >= 5000)
        {
            cout << "超时回收连接" << endl;
            ConnectionPool::getInstance()->recycleConnection(conn);  // 回收
            break;
        }
        int idx = arr[index++];
        switch (idx)
        {
        case 1: 
        {
            conn->work1();  // 模拟工作完6s不操作
            time_start = std::chrono::steady_clock::now();  // 重置计数
            std::this_thread::sleep_for(std::chrono::milliseconds(6000));
        }break;
        case 2: 
        {
            conn->work2();
            time_start = std::chrono::steady_clock::now();
        }break;
        case 3: 
        {
            conn->work3();
            time_start = std::chrono::steady_clock::now();
        } break;
        default:
        {
            label = 0;
            cout << "手动回收连接" << endl;
            ConnectionPool::getInstance()->recycleConnection(conn);  // 回收
        }
        }
    }
}

int main()
{
    int arr[6] = { 2, 3, 2, 3, 3, 4 };
    int arr1[4] = { 3, 2, 3, 1 };
    int arr2[3] = { 2, 2, 1 };
    /*
        如果4结尾，模拟手动归还连接的过程
        如果1结尾，模拟闲置连接超时后自动归还的过程
    */
    std::thread t1(userRequest, arr);
    std::thread t2(userRequest, arr1);
    std::thread t3(userRequest, arr2);

    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}


