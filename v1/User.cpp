#include<iostream>
#include<string>
#include<thread>
#include<functional>
#include<mutex>

#include"Connection.h"
#include"ConnectionPool.h"
#include"User.h"

using namespace std;

//std::mutex mtx;

AbstractUser::AbstractUser() :
	_Connection(nullptr),
	_alivetime(clock()),
	_timeOut(10)
{

}

// 用户发起连接请求
void AbstractUser::toConnect(ConnectionPool* _pConnectPool)
{
	_Connection = _pConnectPool->getConnection(this);
	if (_Connection != nullptr)
	{
		thread t(bind(&AbstractUser::timeoutRecycleConnect, this));
		t.detach();
	}
}

// 用户行为一
void AbstractUser::update()
{
	// 触发一次事件，更新一下时间
	refreshAliveTime();
	if (_Connection != nullptr)
	{
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')",
			"zhang san", 20, "male");
		_Connection->update(sql);
	}
}

// 刷新连接的起始的空闲时间点
void AbstractUser::refreshAliveTime()
{
	_alivetime = clock();
}

// 返回存活的时间
clock_t AbstractUser::getAliceTime() const
{
	return clock() - _alivetime;
}

// 长时间未操作的超时回收
void AbstractUser::timeoutRecycleConnect()
{	
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (getAliceTime() >= 10 * 1000)
		{
			//lock_guard<mutex> lck(mtx);
			cout << "用户" << this << "长时间没有操作，已回收连接\n";
			// 调用其删除器，归还连接
			_Connection.reset();
			_Connection = nullptr;
			break;
		}
	}
}

void AbstractUser::show()
{
	cout << "pdcHelloWorld" << endl;
}

CommonUser::CommonUser():
	AbstractUser()
{

}

VipUser::VipUser() :
	AbstractUser()
{

}