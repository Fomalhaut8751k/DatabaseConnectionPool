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

// �û�������������
void AbstractUser::toConnect(ConnectionPool* _pConnectPool)
{
	_Connection = _pConnectPool->getConnection(this);
}

// �û���Ϊһ
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

// ˢ�����ӵ���ʼ�Ŀ���ʱ���
void AbstractUser::refreshAliveTime()
{
	_alivetime = clock();
}

// ���ش���ʱ��
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