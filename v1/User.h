#pragma once
#include<iostream>
#include"User.h"
#include"ConnectionPool.h"

class AbstractUser
{
public:
	AbstractUser(ConnectionPool* cp);

	virtual void update();

protected:
	shared_ptr<Connection> _conn;
};

// ��ͨ�û�
class CommonUser : public AbstractUser
{
public:
	CommonUser(ConnectionPool* cp);
};


// VIP�û�
class VipUser : public AbstractUser
{
public:
	VipUser(ConnectionPool* cp);
};