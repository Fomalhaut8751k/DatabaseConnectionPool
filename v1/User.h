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

// 普通用户
class CommonUser : public AbstractUser
{
public:
	CommonUser(ConnectionPool* cp);
};


// VIP用户
class VipUser : public AbstractUser
{
public:
	VipUser(ConnectionPool* cp);
};