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

// �û�������������
void AbstractUser::toConnect(ConnectionPool* _pConnectPool)
{
	_Connection = _pConnectPool->getConnection(this);
	if (_Connection != nullptr)
	{
		thread t(bind(&AbstractUser::timeoutRecycleConnect, this));
		t.detach();
	}
}

// �û���Ϊһ
void AbstractUser::update()
{
	// ����һ���¼�������һ��ʱ��
	refreshAliveTime();
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

// ��ʱ��δ�����ĳ�ʱ����
void AbstractUser::timeoutRecycleConnect()
{	
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (getAliceTime() >= 10 * 1000)
		{
			//lock_guard<mutex> lck(mtx);
			cout << "�û�" << this << "��ʱ��û�в������ѻ�������\n";
			// ������ɾ�������黹����
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