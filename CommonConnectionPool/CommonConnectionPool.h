#pragma once
/*
	���ӳع���ģ��
*/
#include<string>
#include<queue>
#include<mutex>
#include<memory>

#include"Connection.h"

class ConnectionPool
{
public:
	// ��ȡ���ӳض���ʵ��
	static ConnectionPool* getConnectionPool();
	// �������ļ��м���������
	bool loadConfigFile();  
	// ���ⲿ�ṩ�ӿڴ����ӳ��л�ȡһ����������
	shared_ptr<Connection> getConnection();  

private:
	ConnectionPool();
	ConnectionPool(const ConnectionPool&) = delete;
	ConnectionPool& operator=(const ConnectionPool&) = delete;

	// �����ڶ������߳��У�ר�Ÿ�������������
	void produceConnectionTask();

	string _ip;  // mysql��ip��ַ
	unsigned short _port;  // mysql�Ķ˿ں� 3306
	string _username;  // mysql��¼�û���
	string _password;  // mysql��¼����
	string _dbname; // mysql���ݿ������
	int _initSize;  // ���ӳصĳ�ʼ������
	int _maxSize;  // ���ӳص����������
	int _maxIdleTime;  // ���ӳ�������ʱ��
	int _connectionTimeout;  // ���ӳػ�ȡ���ӵĳ�ʱʱ��

	queue<Connection*> _connectQueue;  // �洢mysql���ӵĶ���
	mutex _queueMutex;  // ά�����Ӷ��е��̰߳�ȫ������
	atomic_int _connectionCnt;  // ��¼������������connection���ӵ�������
	condition_variable cv;  // ���������������������������̺߳����������̵߳�ͨ��
};