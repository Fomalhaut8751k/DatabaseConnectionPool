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
	_timeOut(10),
	_waiting(false),
	_terminate(false)
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
	while (_Connection == nullptr)
	{
		// ����û�ȷʵ�ڵȴ�
		unique_lock<std::mutex> lck(_pConnectPool->_queueMutex);
		if (_waiting)
		{
			int choice = rand() % 5000000000 + 1;
			// ģ��2%�ĸ����û�ѡ���˳��Ŷ�
			if (choice <= 1) 
			{
				this->_terminate = true;
				
				//_pConnectPool->deleteFromDeque(this);
				
				_Connection.reset();
				_Connection == nullptr;
				break;
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		// ������뵽�����˻�û���˳��Ŷ�
	}
	if (_Connection != nullptr)
	{
		update();
		thread t1(bind(&AbstractUser::timeoutRecycleConnect, this, _pConnectPool));
		t1.join();
	}
	t0.join();
	
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

CommonUser::CommonUser():
	AbstractUser()
{

}

VipUser::VipUser() :
	AbstractUser()
{

}