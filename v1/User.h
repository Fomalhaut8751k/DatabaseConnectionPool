#pragma once
#include<iostream>

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

	virtual void show();

protected:
	shared_ptr<Connection> _Connection;
	clock_t _alivetime;  // ��¼�����û�������ʱ��
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