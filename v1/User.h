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
	AbstractUser();
	
	// �û�������������
	void toConnect(ConnectionPool* _pConnectPool);

	// �û���Ϊһ
	void update();

	// ˢ�����ӵ���ʼ�Ŀ���ʱ���
	void refreshAliveTime();

	// ���ش���ʱ��
	clock_t getAliceTime() const;

	// ��ʱ��δ�����ĳ�ʱ����
	void timeoutRecycleConnect(ConnectionPool* _connectionPool);

	// �û���Ϊ
	void userBehavior();

	virtual void show();

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
	CommonUser();
};


// VIP�û�
class VipUser : public AbstractUser
{
public:
	VipUser();
};