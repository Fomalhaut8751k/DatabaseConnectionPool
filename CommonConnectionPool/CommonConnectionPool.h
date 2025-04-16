#pragma once
/*
	连接池功能模块
*/
#include<string>
#include<queue>
#include<mutex>
#include<memory>

#include"Connection.h"

class ConnectionPool
{
public:
	// 获取连接池对象实例
	static ConnectionPool* getConnectionPool();
	// 从配置文件中加载配置项
	bool loadConfigFile();  
	// 给外部提供接口从连接池中获取一个空闲连接
	shared_ptr<Connection> getConnection();  

private:
	ConnectionPool();
	ConnectionPool(const ConnectionPool&) = delete;
	ConnectionPool& operator=(const ConnectionPool&) = delete;

	// 运行在独立的线程中，专门负责生成新连接
	void produceConnectionTask();

	string _ip;  // mysql的ip地址
	unsigned short _port;  // mysql的端口号 3306
	string _username;  // mysql登录用户名
	string _password;  // mysql登录密码
	string _dbname; // mysql数据库的名称
	int _initSize;  // 连接池的初始连接量
	int _maxSize;  // 连接池的最大连接量
	int _maxIdleTime;  // 连接池最大空闲时间
	int _connectionTimeout;  // 连接池获取连接的超时时间

	queue<Connection*> _connectQueue;  // 存储mysql连接的队列
	mutex _queueMutex;  // 维护连接队列的线程安全互斥锁
	atomic_int _connectionCnt;  // 记录连接所创建的connection连接的总数量
	condition_variable cv;  // 设置条件变量，用于连接生产线程和链接消费线程的通信
};