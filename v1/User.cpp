#include<iostream>
#include<string>
#include<thread>
#include<functional>
#include<mutex>

#include"Connection.h"
#include"ConnectionPool.h"
#include"User.h"

using namespace std;

AbstractUser::AbstractUser(int exit) :
	_Connection(nullptr),
	_alivetime(clock()),
	_timeOut(10),
	_waiting(false),
	_terminate(false),
	_exitOrNot(exit)
{

}

// �û�������������
void AbstractUser::toConnect(ConnectionPool* _pConnectPool)
{
	//_Connection = _pConnectPool->getConnection(this);
	thread t0(
		[&]() -> void {
			_Connection = _pConnectPool->getConnection(this);
		}
	);
	if(_Connection == nullptr)
	{
		thread t1(
			[&]() -> void {
				if (_Connection == nullptr)
				{
					// ÿ5���ж�һ����Ϊ
					std::this_thread::sleep_for(std::chrono::milliseconds(5000));
					if (_Connection == nullptr && _waiting)
					{
						if (_exitOrNot == 1)
						{
							unique_lock<std::mutex> lck(_pConnectPool->_queueMutex);

							this->_terminate = true;
							_Connection.reset();
							_Connection == nullptr;

							_pConnectPool->cv.notify_all();
						}
					}
				}
			}
		);
		// ������뵽�����˻�û���˳��Ŷ�
		t1.join();
		t0.join();
	}

	if (_Connection != nullptr)
	{
		update();
		thread t2(bind(&AbstractUser::timeoutRecycleConnect, this, _pConnectPool));
		t2.join();
	}
	
}

// �û������������󣨲�ʹ�����ӳأ�
void AbstractUser::toConnectWithoutConnectionPool()
{

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
void CommonUser::timeoutRecycleConnect(ConnectionPool* _connectionPool)
{
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (getAliceTime() >= 10 * 1000)
		{
			{
				unique_lock<mutex> lck(_connectionPool->_queueMutex);
				// ������ɾ�������黹����
				cout << "���㲥��";
				cout << "�û�" << this << "\n������ʱ��û�в������ѻ�������\n" << endl;
				cout << "���㲥��";
				cout << "ϵͳ" << ":\n���������ӱ����գ�Ŀǰ���ӳ��л�ʣ: "
					<< _connectionPool->_connectQueue.size() + 1
					<< "����������\n" << endl;
			}
			_Connection.reset();
			_Connection = nullptr;
			_connectionPool->_designedForVip = 0;
			break;
		}
	}
}

void VipUser::timeoutRecycleConnect(ConnectionPool* _connectionPool)
{
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (getAliceTime() >= 10 * 1000)
		{
			{
				unique_lock<mutex> lck(_connectionPool->_queueMutex);
				// ������ɾ�������黹����
				cout << "���㲥��";
				cout << "�û�" << this << "(VIP):\n������ʱ��û�в������ѻ�������\n" << endl;
				cout << "���㲥��";
				cout << "ϵͳ"  << ":\n���������ӱ����գ�Ŀǰ���ӳ��л�ʣ: " 
					<< _connectionPool->_connectQueue.size() + 1
					<< "����������\n" << endl;
			}
			_Connection.reset();
			_Connection = nullptr;
			_connectionPool->_designedForVip = 0;
			break;
		}
	}
}

void AbstractUser::userBehavior()
{
	
}

CommonUser::CommonUser(int exit):
	AbstractUser(exit)
{

}

VipUser::VipUser(int exit) :
	AbstractUser(exit)
{

}