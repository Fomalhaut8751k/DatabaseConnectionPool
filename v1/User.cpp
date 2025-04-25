#include<iostream>
#include<string>
#include<thread>
#include<functional>
#include<mutex>

#include"Connection.h"
#include"ConnectionPool.h"
#include"User.h"

using namespace std;

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
		update();
		thread t1(bind(&AbstractUser::timeoutRecycleConnect, this, _pConnectPool));
		t1.join();
	}
}

// ϸ���û���Ϊһ
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
void AbstractUser::timeoutRecycleConnect(ConnectionPool* _connectionPool)
{	
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (getAliceTime() >= 10 * 1000)
		{
			{
				unique_lock<mutex> lck(_connectionPool->_queueMutex);
				// ������ɾ�������黹����
				cout << "�û�" << this << "��ʱ��û�в������ѻ������ӣ�" <<
					"���ӳ��л�ʣ: " << _connectionPool->_connectQueue.size() + 1
					<< "���������� " << endl;
			}
			_Connection.reset();
			_Connection = nullptr;
			_connectionPool->_designedForVip = 0;
			break;
		}
	}
	//cout << "pdcHelloWorld" << endl;
}

void AbstractUser::userBehavior()
{
	
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