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
	static ConnectionPool* getInstance()  // ����ģʽ������Ψһʵ��
	{
		static ConnectionPool instance;
		return &instance;
	}

	Connection* takeOutFromPool()  // �����ӳ���ȡ��һ�����ӹ��û�ʹ��
	{
		// ��һ��ʱ���ܷ�������Ƿ��п��е�����
		// ����������ʱ�����û�У���...
		auto t0 = std::chrono::steady_clock::now();
		while (1)
		{
			if (std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::steady_clock::now() - t0).count() >= _ConnectionTimeOut)
			{
				throw "request timeout";
			}
			else  // �����û�г�ʱ������⵽�п��е�����
			{
				if (_ConnectionQueue.size() != 0)
				{
					lock_guard<mutex> lck(mtx);  // ����˫���ж�
					if (_ConnectionQueue.size() != 0)
					{
						Connection* _Conn = _ConnectionQueue.front();  // ȡ��ͷ
						_ConnectionQueue.pop();
						return _Conn;
					}
				}
			}
		}
		return nullptr;  // ʵ���ϴ��벻�����е�����
	}

	void recycleConnection(Connection* _pConn)  // ���ղ���Ҫʹ�õ�����
	{
		lock_guard<mutex> lck(mtx);
		_ConnectionQueue.emplace(_pConn);
	}

private:
	ConnectionPool(int init_size = 100, int max_size = 200,
		int connect_timeout = 1000)  // ����ģʽ�����캯��˽�л�
		:_InitSize(init_size),
		_MaxSize(max_size),
		_ConnectionTimeOut(connect_timeout)
	{
		for (int i = 0; i < _InitSize; i++)
		{
			_ConnectionQueue.emplace(new Connection());
		}
	}

	ConnectionPool(const ConnectionPool&) = delete;  // ����ģʽ��ɾ���������캯��
	ConnectionPool& operator=(const ConnectionPool&) = delete;  // ����ģʽ��ɾ����ֵ���

	queue<Connection*> _ConnectionQueue;  // ������ӵĶ��У������Ҫ��֤�̰߳�ȫ(��ӳ���)
	const int _InitSize;  // ��ʼ������
	const int _MaxSize;  // ���������
	const int _ConnectionTimeOut;  // ������ӳ�ʱʱ��
};