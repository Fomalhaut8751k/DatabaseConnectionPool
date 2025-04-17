#pragma once
/*
	ʵ��MySQL���ݿ�Ĳ���
*/
#include<iostream>
#include <mysql.h>
#include <string>

using namespace std;
#include "public.h"


class Connection
{
public:
	// ��ʼ�����ݿ�����
	Connection();

	// �ͷ����ݿ�������Դ
	~Connection();

	// �������ݿ�
	bool connect(string ip, unsigned short port, string user, string password,
		string dbname);

	// ���²��� insert��delete��update
	bool update(string sql);

	// ��ѯ���� select
	MYSQL_RES* query(string sql);

	// ˢ�����ӵ���ʼ�Ŀ���ʱ���
	void refreshAliveTime();
	/*
		������Ӳ������У������̵߳������ߵ��������̣߳��û��黹(����)���߳�
	*/

	// ���ش���ʱ��
	clock_t getAliceTime() const;

private:
	MYSQL* _conn; // ��ʾ��MySQL Server��һ������
	clock_t _alivetime;  // ��¼�������״̬��Ĵ��ʱ��
};