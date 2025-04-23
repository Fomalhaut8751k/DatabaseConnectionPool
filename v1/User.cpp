#include<iostream>
#include<string>
#include<thread>
#include"Connection.h"
#include"ConnectionPool.h"
#include"User.h"

using namespace std;

AbstractUser::AbstractUser():
	_Connection(nullptr)
{
	
}

// 用户发起连接请求
void AbstractUser::toConnect(ConnectionPool* _pConnectPool)
{
	_Connection = _pConnectPool->getConnection(this);
}

// 用户行为一
void AbstractUser::update()
{
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