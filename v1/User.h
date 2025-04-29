#pragma once
#include<iostream>
#include<mutex>
#include<condition_variable>

#include"ConnectionPool.h"
#include"Connection.h"

class ConnectionPool;

class AbstractUser
{
public:
	AbstractUser(int exit = 0);
	
	// �û�������������
	void toConnect(ConnectionPool* _pConnectPool);

	// �û������������󣨲�ʹ�����ӳأ�
	void toConnectWithoutConnectionPool();

	// �û���Ϊһ
	void update();

	// ˢ�����ӵ���ʼ�Ŀ���ʱ���
	void refreshAliveTime();

	// ���ش���ʱ��
	clock_t getAliceTime() const;

	// ��ʱ��δ�����ĳ�ʱ����
	virtual void timeoutRecycleConnect(ConnectionPool* _connectionPool) = 0;

	// �û���Ϊ
	void userBehavior();

	// �û��ȴ����
	atomic_bool _waiting;

	// �߳���ֹ���
	atomic_bool _terminate;

	int _exitOrNot;

protected:
	shared_ptr<Connection> _Connection;
	clock_t _alivetime;  // ��¼�����û�������ʱ��
	int _timeOut;
	std::mutex _userMtx;
};

// ��ͨ�û�
class CommonUser : public AbstractUser
{
public:
	CommonUser(int exit = 0);

	void timeoutRecycleConnect(ConnectionPool* _connectionPool);
};


// VIP�û�
class VipUser : public AbstractUser
{
public:
	VipUser(int exit = 0);

	void timeoutRecycleConnect(ConnectionPool* _connectionPool);
};