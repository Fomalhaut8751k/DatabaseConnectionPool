#include<iostream>
#include<string>
#include"Connection.h"
#include"ConnectionPool.h"
#include"User.h"

using namespace std;

AbstractUser::AbstractUser(ConnectionPool* cp):
	_conn(cp->getConnection(this))
{

}

void AbstractUser::update()
{
	char sql[1024] = { 0 };
	sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')",
		"zhang san", 20, "male");
	_conn->update(sql);
}

CommonUser::CommonUser(ConnectionPool* cp):
	AbstractUser(cp)
{

}

VipUser::VipUser(ConnectionPool* cp) :
	AbstractUser(cp)
{

}