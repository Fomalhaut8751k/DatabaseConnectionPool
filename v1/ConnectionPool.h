#pragma once
/*
	���ӳع���ģ��
*/
#include<string>
#include<queue>
#include<mutex>
#include<memory>
#include<deque>

#include"Connection.h"
#include"User.h"

class ConnectionPool
{
public:
	// ��ȡ���ӳض���ʵ��
	static ConnectionPool* getConnectionPool();

	// �������ļ��м���������
	bool loadConfigFile();

	// ���ⲿ�ṩ�ӿڴ����ӳ��л�ȡһ����������
	shared_ptr<Connection> getConnection(AbstractUser* _abUser);

	// ɾ��deque���Ѿ������˳��Ŷӵ��û�
	void deleteFromDeque(AbstractUser* _abUser);

	queue<Connection*> _connectQueue;  // �洢mysql���ӵĶ���
	atomic_bool _priorUser;
	mutex _queueMutex;  // ά�����Ӷ��е��̰߳�ȫ������
	atomic_int _designedForVip;  // �ж��Ƿ���Ϊvipר������������

private:
	ConnectionPool();
	ConnectionPool(const ConnectionPool&) = delete;
	ConnectionPool& operator=(const ConnectionPool&) = delete;

	// �����ڶ������߳��У�ר�Ÿ�������������
	void produceConnectionTask();

	// �����ڶ������߳��У���������п�������ʱ�䳬��maxIdleTime�����ӵ��ͷ�
	void recycleConnectionTask();

	string _ip;  // mysql��ip��ַ
	unsigned short _port;  // mysql�Ķ˿ں� 3306
	string _username;  // mysql��¼�û���
	string _password;  // mysql��¼����
	string _dbname; // mysql���ݿ������
	int _initSize;  // ���ӳصĳ�ʼ������
	int _maxSize;  // ���ӳص����������
	int _maxIdleTime;  // ���ӳ�������ʱ��
	//int _connectionTimeout;  // ���ӳػ�ȡ���ӵĳ�ʱʱ��

	atomic_int _connectionCnt;  // ��¼������������connection���ӵ�������
	condition_variable cv;  // ���������������������������̺߳����������̵߳�ͨ��

	deque<CommonUser*> commonUserDeque;  // ��ͨ�û����Ŷ��б�
	deque<VipUser*> vipUserDeque;  // vip�û����Ŷ��б�


	atomic_bool _produceForVip;  // �ж��Ƿ���(initSize, maxSize)�����·���������

};
