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
	_designedForVip = 0;  // �ж��Ƿ���Ϊvipר������������
	_produceForVip = false;  // �ж��Ƿ���(initSize, maxSize)�����·���������
	_priorUser = true;

	// ����������
	if (!loadConfigFile())
	{
		return;
	}

	// ������ʼ����������
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		// ˢ�¼�ʱ
		p->refreshAliveTime();
		_connectQueue.push(p);
		_connectionCnt++;
	}

	// ����һ���µ��̣߳���Ϊ���ӵ�������
	thread produce(&ConnectionPool::produceConnectionTask, this);
	produce.detach();

	// ����һ���µ��̣߳���������п�������ʱ�䳬��maxIdleTime�����ӵ��ͷ�,ֻ����initsize����������
	thread timekeeper(&ConnectionPool::recycleConnectionTask, this);
	timekeeper.detach();
}

// �̰߳�ȫ����������ģʽ�ӿ�
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool instance;
	return &instance;
}

// �������ļ��м���������
bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist!");
		return false;
	}
	while (!feof(pf))  // û�ж���ĩβ(�ͼ�����)
	{
		char line[1024] = {};
		fgets(line, 1024, pf); // �ȶ�һ��
		string str = line;
		int idx = str.find("=", 0);  // ����ip=127.0.0.1����key��value�ķֽ��
		if (idx == -1)  // ע�ͻ���Ч��������
		{
			continue;
		}
		int endidx = str.find("\n", idx);  // ��idx��ʼ�ҽ�β

		string key = str.substr(0, idx);  // ��0��ʼ����Ϊidx����key���ֶ�
		string value = str.substr(idx + 1, endidx - idx - 1);  // ͬ�ϣ�value���ֶ�

		//cout << key << ": " << value << endl;

		if (key == "ip")  // mysql��ip��ַ
		{
			_ip = value;
		}
		else if (key == "port")  // mysql�Ķ˿ں� 3306
		{
			_port = atoi(value.c_str());
		}
		else if (key == "username")  // mysql��¼�û���
		{
			_username = value;
		}
		else if (key == "password")  // mysql��¼����
		{
			_password = value;
		}
		else if (key == "dbname")  // mysql���ݿ������
		{
			_dbname = value;
		}
		else if (key == "initSize")  // ���ӳصĳ�ʼ������
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize")  // ���ӳص����������
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime")  // ���ӳ�������ʱ��
		{
			_maxIdleTime = atoi(value.c_str());
		}
	}
	return true;
}


// ���ⲿ�ṩ�ӿڴ����ӳ��л�ȡһ����������
shared_ptr<Connection> ConnectionPool::getConnection(AbstractUser* _abUser)
{
	unique_lock<mutex> lock(_queueMutex);
	
	cv.wait(lock,
		[&]() -> bool {
			return _priorUser == true;
		}
	);
	_priorUser = false;

	// �ж��������ӵ�����ͨ�û�����vip�û�
	if (dynamic_cast<CommonUser*>(_abUser) != nullptr)
	{	// �������ͨ�û�

		// ֻ�ж�һ�Σ�û�о��Ŷ�
		if (_connectQueue.empty())  
		{
			commonUserDeque.push_back(dynamic_cast<CommonUser*>(_abUser));
			cout << "���㲥��";
			cout << "�û�" << _abUser << ":\n���������Ŷ���......"
				<< "ǰ�滹��" << vipUserDeque.size() + commonUserDeque.size() << "��\n" << endl;
			_priorUser = true;

			cv.notify_all();
			_abUser->_waiting = true;  // ��ʾ�û����ڵȴ�״̬

			cv.wait(lock, [&]() -> bool {
				// �п������� -> vipͨ��û�������Ŷ� -> ���Ƕ�ͷ
				return (_abUser->_terminate == true) || (
						(!_connectQueue.empty())
						&& vipUserDeque.empty()
						&& commonUserDeque.front() == _abUser
						&& (_designedForVip == 0)
					);
				}
			);
			// ���������������һֱ�ȣ�ֱ��������
			commonUserDeque.pop_front();
		}
	}
	else
	{   // �����vip�û�

		if (_connectQueue.empty())
		{
			// ����Ѿ�������maxSize����ʹ��vipҲ�ùԹ��Ŷ�
			if (_connectionCnt >= _maxSize)
			{
				vipUserDeque.push_back(dynamic_cast<VipUser*>(_abUser));
				cout << "���㲥��";
				cout << "�û�" << _abUser << "(VIP):\n���������Ŷ���......Ϊ������vipͨ����"
					<< "ǰ�滹��" << vipUserDeque.size() - 1 << "��\n" << endl;

				_priorUser = true;

				cv.notify_all();
				_abUser->_waiting = true;  // ��ʾ�û����ڵȴ�״̬

				cv.wait(lock, 
					[&]() -> bool {
					// ���ȼ����п������� -> ���Ƕ�ͷ
						return (_abUser->_terminate == true) || (
								(!_connectQueue.empty())
								&& vipUserDeque.front() == _abUser
							);
					}
				);
				// ���������������һֱ�ȣ�ֱ��������
				vipUserDeque.pop_front();
			}
			// ���û�г���������Ϊvip�û�����ר��������
			else
			{
				// �ȴ������������µ�����
				_produceForVip = true;
				_designedForVip = 1;
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

	if (_abUser->_terminate)
	{
		return nullptr;
	}

	shared_ptr<Connection> sp(_connectQueue.front(),
		[&](Connection* pcon) { 
			{
				unique_lock<mutex> lock(_queueMutex);
				// ˢ�¼�ʱ
				pcon->refreshAliveTime();

				_connectQueue.push(pcon);
			}
			// ���˿�����֮���Ӧ��....
			cv.notify_all();
		}
	);   // ȡ��ͷ

	if (dynamic_cast<CommonUser*>(_abUser) != nullptr)
	{
		cout << "���㲥��";
		cout << "�û�" << _abUser << ":\n�����ɹ����뵽������\n" << endl;
	}
	else
	{
		cout << "���㲥��";
		cout << "�û�" << _abUser << "(VIP):\n�����ɹ����뵽������\n" << endl;
	}
		
	_priorUser = true;
	_connectQueue.pop();  // Ȼ�󵯳�
	cv.notify_all();  // ���Ѻ��֪ͨ

	return sp;
}

// �����ڶ������߳��У�ר�Ÿ���Ϊvip�û�����������
void ConnectionPool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);
		// ���в��գ��˴������߳̽���ȴ�״̬���ȴ�ָ���������㲢������

		while (!_connectQueue.empty())
		{
			_designedForVip = 2;
			cv.notify_all();
			cv.wait(lock, [&]() -> bool {
					// ֻ��Ӧ�ڿɴ��������µ�vip�û��Ĵ�������
					return (_produceForVip == true)
						&& (_designedForVip == 1);
				}
			);
		}
		cout << "���㲥��";
		cout << "ϵͳ:\n����Ϊvip�û������µ�����....\n"  << endl;
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		// ˢ�¼�ʱ
		p->refreshAliveTime();

		_connectQueue.push(p);
		_connectionCnt++;
		
		_produceForVip = false;
		
		/*_designedForVip = 2;*/

		cv.notify_all();  // ֪ͨ�������̣߳���������������
	}
}

// �����ڶ������߳��У���������п�������ʱ�䳬��maxIdleTime�����ӵ��ͷ�
void ConnectionPool::recycleConnectionTask()
{
	/*
		ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж�������ӻ���
	*/
	for (;;)
	{
		// ͨ��sleepģ�ⶨʱЧ��
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		// ɨ���������У��ͷŶ��������
		unique_lock<mutex> lock(_queueMutex);
		
		while ((_connectionCnt > _initSize) && (!_connectQueue.empty()))  // ��Ȼ�Ǵ�ͷ��ʼɾ
		{
			Connection* p = _connectQueue.front();
			if (p->getAliceTime() >= (_maxIdleTime * 1000))
			{
				_connectQueue.pop();
				_connectionCnt--;
				cout << "���㲥��";
				cout << "ϵͳ:\n���������ӳ�ʱ�����ͷţ�Ŀǰ���ӳػ�ʣ��" 
					<< _connectQueue.size() << "��\n" << endl;
				delete p;  // �ͷ�����
			}
			else
			{
				break;  // �����ͷ��û��ʱ���Ǻ����Ҳ���ᳬʱ
			}
		}
	}
}

void ConnectionPool::deleteFromDeque(AbstractUser* _abUser)
{
	//unique_lock<std::mutex> lck(_queueMutex);

	if (dynamic_cast<CommonUser*>(_abUser) != nullptr)
	{
		CommonUser* _user = dynamic_cast<CommonUser*>(_abUser);
		deque<CommonUser*>::iterator it = std::find(commonUserDeque.begin(), commonUserDeque.end(), _user);
		if (it != commonUserDeque.end())
		{
			commonUserDeque.erase(it);
			cout << "���㲥��";
			cout << "�û� " << _abUser << ":\n�����˳����Ŷ�, �û����л���: " << commonUserDeque.size()
				<< "\n" << endl;
		}
		else
		{
			throw "Error: This user is not in the queue";
		}
	}
	else
	{
		VipUser* _user = dynamic_cast<VipUser*>(_abUser);
		deque<VipUser*>::iterator it = std::find(vipUserDeque.begin(), vipUserDeque.end(), _user);
		if (it != vipUserDeque.end())
		{
			vipUserDeque.erase(it);
			cout << "���㲥��";
			cout << "�û�" << _abUser << "(VIP):\n�����˳����Ŷӣ�VIP�û����л���: " << vipUserDeque.size() << endl;
		}
		else
		{
			cout << "pdcHelloWorld" << endl;
			throw "Error: This user is not in the queue";
		}
	}
	cv.notify_all();

}