#pragma once
#include<iostream>

#include<queue>
#include<mutex>
#include<thread>

#include "Connection.h"

using namespace std;


std::mutex mtx;

class ConnectionPool
{
public:
	static ConnectionPool* getInstance()  // 单例模式，返回唯一实例
	{
		static ConnectionPool instance;
		return &instance;
	}

	Connection* takeOutFromPool()  // 从连接池中取出一个连接供用户使用
	{
		// 在一段时间能反复检测是否有空闲的连接
		// 如果超过这段时候后还是没有，就...
		auto t0 = std::chrono::steady_clock::now();
		while (1)
		{
			if (std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::steady_clock::now() - t0).count() >= _ConnectionTimeOut)
			{
				throw "request timeout";
			}
			else  // 如果还没有超时，并检测到有空闲的连接
			{
				if (_ConnectionQueue.size() != 0)
				{
					lock_guard<mutex> lck(mtx);  // 锁加双重判定
					if (_ConnectionQueue.size() != 0)
					{
						Connection* _Conn = _ConnectionQueue.front();  // 取出头
						_ConnectionQueue.pop();
						return _Conn;
					}
				}
			}
		}
		return nullptr;  // 实际上代码不会运行到这里
	}

	void recycleConnection(Connection* _pConn)  // 回收不需要使用的连接
	{
		lock_guard<mutex> lck(mtx);
		_ConnectionQueue.emplace(_pConn);
	}

private:
	ConnectionPool(int init_size = 100, int max_size = 200,
		int connect_timeout = 1000)  // 单例模式，构造函数私有化
		:_InitSize(init_size),
		_MaxSize(max_size),
		_ConnectionTimeOut(connect_timeout)
	{
		for (int i = 0; i < _InitSize; i++)
		{
			_ConnectionQueue.emplace(new Connection());
		}
	}

	ConnectionPool(const ConnectionPool&) = delete;  // 单例模式，删除拷贝构造函数
	ConnectionPool& operator=(const ConnectionPool&) = delete;  // 单例模式，删除赋值语句

	queue<Connection*> _ConnectionQueue;  // 存放连接的队列，其操作要保证线程安全(入队出队)
	const int _InitSize;  // 初始连接量
	const int _MaxSize;  // 最大连接量
	const int _ConnectionTimeOut;  // 最大连接超时时间
};