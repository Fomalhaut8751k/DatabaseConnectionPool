﻿#include<iostream>
#include<string>
#include<thread>
#include<functional>
#include<atomic>
#include<condition_variable>
#include"CommonConnectionPool.h"

using namespace std;

ConnectionPool::ConnectionPool() 
{
	// 加载配置项
	if (!loadConfigFile())
	{
		return;
	}

	// 创建初始数量的连接
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		_connectQueue.push(p);
		_connectionCnt++;
	}
	
	// 启动一个新的线程，作为连接的生产者
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	/*
		由于produceConnectionTask是成员函数，它的调用依赖于对象，需要把
		调用需要的this指针通过绑定器绑定上
	*/
}

// 线程安全的懒汉单例模式接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool instance;
	return &instance;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist!");
		return false;
	}
	while (!feof(pf))  // 没有读到末尾(就继续读)
	{
		char line[1024] = {};
		fgets(line, 1024, pf); // 先读一行
		string str = line;
		int idx = str.find("=");  // 例如ip=127.0.0.1，找key和value的分界点
		if (idx == -1)  // 注释或无效的配置项
		{
			continue;
		}
		int endidx = str.find("\n", idx);  // 从idx开始找结尾

		string key = str.substr(0, idx);  // 从0开始长度为idx就是key的字段
		string value = str.substr(idx + 1, endidx - idx - 1);  // 同上，value的字段

		if (key == "ip")  // mysql的ip地址
		{
			_ip = value;
		}
		else if (key == "port")  // mysql的端口号 3306
		{
			_port = atoi(value.c_str());
		} 
		else if (key == "username")  // mysql登录用户名
		{
			_username = value;
		}
		else if (key == "password")  // mysql登录密码
		{
			_password = value;
		}
		else if (key == "dbname")  // mysql数据库的名称
		{
			_dbname = value;
		}
		else if (key == "initSize")  // 连接池的初始连接量
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize")  // 连接池的最大连接量
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime")  // 连接池最大空闲时间
		{
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut")  // 连接池获取连接的超时时间
		{
			_connectionTimeout = atoi(value.c_str());
		}
	}
	return true;
}

// 给外部提供接口从连接池中获取一个空闲连接
shared_ptr<Connection> ConnectionPool::getConnection()
{
	/*
		通过一个智能指针，当用户不用时就自动析构，然后重定义智能指针的
		删除资源的方式(删除器)
	*/
	unique_lock<mutex> lock(_queueMutex);
	while(_connectQueue.empty())
	{
		/*
			使当前线程阻塞一段指定的时间或直到另一个线程调用
			notify_one 或 notify_all，以先发生者为准。
			——要么被notify唤醒，要么超时被唤醒
		*/
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{  // 如果是超时被唤醒的
			LOG("获取空闲连接超时...获取连接失败");
			
			//if (_connectQueue.empty())  // 如果还是空的，那就没有可以申请的空闲连接
			//{
			//	LOG("获取空闲连接超时...获取连接失败");
			//	return nullptr;
			//}

			return nullptr;
		}
	}
	
	/*
		shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于调用
		connection的析构函数，connection就被close掉了。
		这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue中
	*/

	shared_ptr<Connection> sp(_connectQueue.front(),
		[&](Connection* pcon)->void {  // &: 捕获外部变量的方式
			// 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
			unique_lock<mutex> lock(_queueMutex);
			_connectQueue.push(pcon);
		}
	);   // 取队头
	_connectQueue.pop();  // 然后弹出
	if (_connectQueue.empty())  // 消费了最后一个连接，就通知生产者生产，也可以不加判断
	{
		cv.notify_all();
	}
	

	return sp;
}

// 运行在独立的线程中，专门负责生成新连接
void ConnectionPool::produceConnectionTask()
{
	for (;;)  
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectQueue.empty())  
		{
			cv.wait(lock);  // 队列不空，此处生产线程进入等待状态
		}
		if (_connectionCnt < _maxSize)  // 只有小于最大数量是才会创建
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			_connectQueue.push(p);
			_connectionCnt++;
		}
	
		cv.notify_all();  // 通知消费者线程，可以消费连接了
	}
}