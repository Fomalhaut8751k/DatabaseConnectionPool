#include<iostream>
#include<thread>
#include<mutex>

#include "ConnectionPool.h"
#include "Connection.h"

using namespace std;

/* �򵥵Ŀ��ʵ�� */

void userRequest(int arr[])  // �û����̺߳���
{
    auto time_start = std::chrono::steady_clock::now();
    // ������̳���10sû��ִ����ز�������������ӣ�����ˢ�¼�ʱ
    
    // �����ӳ��л�ȡһ������
    cout << "��ȡ����" << endl;
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
            cout << "��ʱ��������" << endl;
            ConnectionPool::getInstance()->recycleConnection(conn);  // ����
            break;
        }
        int idx = arr[index++];
        switch (idx)
        {
        case 1: 
        {
            conn->work1();  // ģ�⹤����6s������
            time_start = std::chrono::steady_clock::now();  // ���ü���
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
            cout << "�ֶ���������" << endl;
            ConnectionPool::getInstance()->recycleConnection(conn);  // ����
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
        ���4��β��ģ���ֶ��黹���ӵĹ���
        ���1��β��ģ���������ӳ�ʱ���Զ��黹�Ĺ���
    */
    std::thread t1(userRequest, arr);
    std::thread t2(userRequest, arr1);
    std::thread t3(userRequest, arr2);

    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}


