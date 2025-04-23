#include<iostream>
#include<string>
#include<thread>
#include<functional>
#include<atomic>
#include<condition_variable>

#include"ConnectionPool.h"
#include"Connection.h"
#include"User.h"

using namespace std;

ConnectionPool::ConnectionPool()
{
	_designedForVip = 0;  // 判断是否是为vip专门生产的连接
	_produceForVip = false;  // 判断是否在(initSize, maxSize)条件下发来的请求

	// 加载配置项
	if (!loadConfigFile())
	{
		return;
	}

	// 创建初始数量的连接
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		// 刷新计时
		p->refreshAliveTime();
		_connectQueue.push(p);
		_connectionCnt++;
	}

	// 启动一个新的线程，作为连接的生产者
	thread produce(&ConnectionPool::produceConnectionTask, this);
	produce.detach();

	// 启动一个新的线程，管理队列中空闲连接时间超过maxIdleTime的连接的释放,只保留initsize个空闲连接
	thread timekeeper(&ConnectionPool::recycleConnectionTask, this);
	timekeeper.detach();
}

// 线程安全的懒汉单例模式接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool instance;
	return &instance;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist!");
		return false;
	}
	while (!feof(pf))  // 没有读到末尾(就继续读)
	{
		char line[1024] = {};
		fgets(line, 1024, pf); // 先读一行
		string str = line;
		int idx = str.find("=", 0);  // 例如ip=127.0.0.1，找key和value的分界点
		if (idx == -1)  // 注释或无效的配置项
		{
			continue;
		}
		int endidx = str.find("\n", idx);  // 从idx开始找结尾

		string key = str.substr(0, idx);  // 从0开始长度为idx就是key的字段
		string value = str.substr(idx + 1, endidx - idx - 1);  // 同上，value的字段

		//cout << key << ": " << value << endl;

		if (key == "ip")  // mysql的ip地址
		{
			_ip = value;
		}
		else if (key == "port")  // mysql的端口号 3306
		{
			_port = atoi(value.c_str());
		}
		else if (key == "username")  // mysql登录用户名
		{
			_username = value;
		}
		else if (key == "password")  // mysql登录密码
		{
			_password = value;
		}
		else if (key == "dbname")  // mysql数据库的名称
		{
			_dbname = value;
		}
		else if (key == "initSize")  // 连接池的初始连接量
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize")  // 连接池的最大连接量
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime")  // 连接池最大空闲时间
		{
			_maxIdleTime = atoi(value.c_str());
		}
	}
	return true;
}


// 给外部提供接口从连接池中获取一个空闲连接
shared_ptr<Connection> ConnectionPool::getConnection(AbstractUser* _abUser)
{
	unique_lock<mutex> lock(_queueMutex);
	// 判断申请连接的是普通用户还是vip用户
	if (dynamic_cast<CommonUser*>(_abUser) != nullptr)
	{	// 如果是普通用户

		// 只判断一次，没有就排队
		if (_connectQueue.empty())  
		{
			commonUserDeque.push_back(dynamic_cast<CommonUser*>(_abUser));

			cout << "用户" << _abUser << "正在排队中......"
				<< "前面还有" << vipUserDeque.size() + commonUserDeque.size() << "人" << endl;
			cv.wait(lock, [&]() -> bool {
				// 有空闲连接 -> vip通道没有人在排队 -> 我是队头
				return (!_connectQueue.empty())
					&& vipUserDeque.empty()
					&& commonUserDeque.front() == _abUser
					&& (_designedForVip == 0);
				}
			);
			// 三个条件不满足就一直等，直到都满足
			commonUserDeque.pop_front();
		}
	}
	else
	{   // 如果是vip用户

		if (_connectQueue.empty())
		{
			// 如果已经超过了maxSize，即使是vip也得乖乖排队
			if (_connectionCnt >= _maxSize)
			{
				vipUserDeque.push_back(dynamic_cast<VipUser*>(_abUser));
				cout << "用户" << _abUser << "正在排队中......正在为您开启vip通道，"
					<< "前面还有" << vipUserDeque.size() << "人" << endl;
				cv.wait(lock, 
					[&]() -> bool {
					// 优先级：有空闲连接 -> 我是队头
						return (!_connectQueue.empty())
							&& vipUserDeque.front() == _abUser;
					}
				);
				// 两个条件不满足就一直等，直到都满足
				vipUserDeque.pop_front();
			}
			// 如果没有超过，可以为vip用户申请专属的连接
			else
			{
				// 等待生产者生成新的连接
				_produceForVip = 1;  
				cv.notify_all();  
				cv.wait(lock, 
					[&]() -> bool {
						return (!_connectQueue.empty()) 
							&& (_designedForVip == 2);
					}
				);
				_designedForVip = 0;
			}
		}
	}

	shared_ptr<Connection> sp(_connectQueue.front(),
		[&](Connection* pcon) { 
			unique_lock<mutex> lock(_queueMutex);
			// 刷新计时
			pcon->refreshAliveTime();

			_connectQueue.push(pcon);
		}
	);   // 取队头

	cout << "用户" << _abUser << "成功申请到了连接" << endl;

	cout << _connectQueue.size() << " " << _designedForVip << " " << _produceForVip << " "
		<< commonUserDeque.size() << " " << vipUserDeque.size() << endl;

	_connectQueue.pop();  // 然后弹出
	cv.notify_all();  // 消费后就通知

	return sp;
}

// 运行在独立的线程中，专门负责为vip用户生成新连接
void ConnectionPool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);

		// 队列不空，此处生产线程进入等待状态，等待指定条件满足并被唤醒
		while (!_connectQueue.empty())
		{
			cv.wait(lock, [&]() -> bool {
					// 只响应在可创建条件下的vip用户的创建请求
					return _produceForVip == true;
				}
			);
		}
		cout << "为vip用户创建新的连接...."  << endl;
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		// 刷新计时
		p->refreshAliveTime();

		_connectQueue.push(p);
		_connectionCnt++;
		
		_produceForVip = false;
		_designedForVip = 2;

		cv.notify_all();  // 通知消费者线程，可以消费连接了
	}
}

// 运行在独立的线程中，管理队列中空闲连接时间超过maxIdleTime的连接的释放
void ConnectionPool::recycleConnectionTask_v1()
{
	/*
		如果连接池中空闲的连接数量超过initsize,并且其中存在(归还到queue)连接的空闲时间
		超过maxidletime，那么就把这些空闲的连接回收掉，最终只保留initsize个

		从queue的头取出，从尾部放回，那么空闲时间应该是从队头到队尾逐渐递减的
		如果当前队列内空闲连接的数量大于initsize（while(_connectionCnt > _initSize)）
		判断队头，如果队头的空闲时间大于最大空闲时间，就把它回收

		如何计算一个线程的空闲时间？cv.wait_for()?

		其他线程操作：
		1. getConnection
		2. produceConnectionTask(线程的同步通信)
		3. 归还连接到connection
	*/
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt <= _initSize)
		{
			// 消费者通知该线程他getConnection——取走了队头的连接
			// 就要重新计算新队头的时间——从零开始？不从零开始
			// 如果不从零开始，就得记录队列中每一个的空闲时间
			if (cv_status::timeout == cv.wait_for(lock, chrono::seconds(_maxIdleTime)))
			{	// 如果是超时被唤醒的
				if (_connectionCnt <= _initSize)
				{
					_connectQueue.front()->~Connection();
					_connectQueue.pop();
				}
			}
			else
			{  // 如果是连接被申请走了后被唤醒的
				continue;  // 那就重新开始计时
			}
		}
	}
}

void ConnectionPool::recycleConnectionTask()
{
	/*
		扫描超过maxIdleTime时间的空闲连接，进行多余的连接回收
	*/
	for (;;)
	{
		// 通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		// 扫描整个队列，释放多余的连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)  // 依然是从头开始删
		{
			Connection* p = _connectQueue.front();
			if (p->getAliceTime() >= (_maxIdleTime * 1000))
			{
				_connectQueue.pop();
				_connectionCnt--;
				delete p;  // 释放连接
			}
			else
			{
				break;  // 如果队头都没超时，那后面的也不会超时
			}
		}
	}
}