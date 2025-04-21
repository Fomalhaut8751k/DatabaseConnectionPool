/* Ԥ��֪ʶ��
	1. ����ģʽ
	2. queue��������
	3. c++11���̱߳�̣��̻߳��⣬�߳�ͬ��ͨ�ź�unique_lock
	4. ����CAS��ԭ��ģ��
	5. ����ָ��shared_ptr
	6. lambda���ʽ
	7. ������-�������߳�ģ��
	8. MySql���ݿ���
*/

#include<iostream>
using namespace std;

// ###### ����ģʽ #############################################################
#if 0
class SingleInstance
{
public:
	// ��ȡ���Ψһʵ������Ľӿڷ���
	static SingleInstance* getInstacne()
	{
		/*
			��̬�ľֲ�����������������׶Σ��ڴ�ʹ��������ݶ���
			���ǳ�ʼ���ǵȵ����е��ô���ʱ�ų�ʼ����
			��̬�ֲ�����ִֻ��һ�γ�ʼ��
			������̬�ֲ������ĳ�ʼ�����ڻ��ָ�����Ѿ��Զ�����̻߳���ָ����
		*/
		static SingleInstance instance;
		return& instance;
	}

	int a;
	int b;

private:
	SingleInstance()  // ���캯��˽�л�
	{
		a = 10;
		b = 20;
	}
	SingleInstance(const SingleInstance&) = delete;
	SingleInstance& operator=(const SingleInstance&) = delete;

};

class A
{
public:
	int returnValueA(int value)
	{
		static int a = value;  // ��ʼ��
		return a;
	}
	int returnValueB(int value)
	{
		b = value;  // ��ֵ
		return b;
	}

	static int b;
};
int A::b = 1;  // ���������������ʼ��

void test1()
{
	A a;
	for (int i = 0; i < 10; ++i)
	{
		// ��̬��Ա����a��ͨ��returnValue��γ�ʼ����������Ľ�����ǵ�һ�ε�0
		cout << a.returnValueA(i) << " ";
	}cout << endl;

	for (int i = 0; i < 10; ++i)
	{
		// ���Ǹ�ֵ�����ǳ�ʼ�������b��ֵ�����Ÿı�
		cout << a.returnValueB(i) << " ";
	}cout << endl;
}

int main()
{
	// ����ģʽ
	test1();

	// �����������д�ɾ�̬��������������������ȥ����
	SingleInstance* s1 = SingleInstance::getInstacne();
	SingleInstance* s2 = SingleInstance::getInstacne();

	cout << "ͨ��s1����a��" << s1->a << endl;
	cout << "ͨ��s1����b��" << s1->b << endl;
	cout << "ͨ��s2����a��" << s2->a << endl;
	cout << "ͨ��s2����b��" << s2->b << endl;
	
	return 0;
}
#endif

// ###### queue���в��� ######################################################
#if 0
#include<queue>
/*  ���ȣ�queue��һ������������
	�������ײ�û���Լ������ݽṹ����������һ�������ķ�װ�����ķ�����
	ȫ���ɵײ�����������ʵ�ֵģ�û��ʵ���Լ��ĵ�������
	push��ӣ�pop���ӣ�front�鿴��ͷԪ�أ�back�鿴��βԪ�أ�empty�пգ�sizeԪ�ظ���
	������deque
*/

int main()
{
	// queue<int> que = { 1, 2, 3, 4 ,5 };
	queue<int> que;
	for (int i = 0; i < 5; i++)
	{
		que.push(i + 1);
	}
	// for(auto item: que) û�е��������޷���������
	while (!que.empty())
	{
		cout << que.front() << endl;
		que.pop();
	}

	return 0;
}
#endif
// ###### c++11���̲߳��� ######################################################
#if 0
#include<thread>  // ��Ҫʡ��std::

void threadHandle01()
{
	for (auto i = 0; i < 10; i++)
	{
		std::cout << i << std::endl;
	}
}

void threadHandle02()
{
	for (auto i = 0; i < 100; i++)
	{
		std::cout << i << std::endl;
	}
}

int main()
{
	std::thread t1(threadHandle01);  // �̶߳�����һ���̺߳���
	std::thread t2(threadHandle02);
	/*
		һ������£�Ҫ�����߳������߳̽���֮�����������ᱨ��
		���������²���ʵ�֣�
	*/
	t1.join();  // ���߳�ִ�е��˴����ȴ����߳�t1ִ������ٽ���ȥִ��
	t2.detach();  // �����̷߳��룬���̲߳���Ҫ����(���߳�����������û����û��)

	// һЩ�߳��еĲ���
	std::this_thread::sleep_for(std::chrono::seconds(2));  // ��������

	return 0;
}
#endif
// ###### �̼߳�Ļ������ ####################################################
#if 0
#include<thread>
#include<list>
#include<mutex>
/*
	�����п��ܴ���һЩ�̲߳���ȫ�����������ڶ����ȫ�ֱ���Blood�ĸı�(Blood--)
	Blood--�ڻ���а���3�����������ܳ��֣�����Blood��20�����߳�һ����ִ��--��δִ��
	���ʱ����ʱ�̶߳��Ѿ����룬Blood����20�����������̷ֱ߳�ִ��--�Ľ��ȴ��19��

	�ؼ�������lock_guard<std::mutex> lock(mtx);
	ȡ��������Ľ��̽����������������̶߳������⣬�ȵ�������(��������������)
	һ������+˫���ж�
*/

int Blood = 1000;  // boss��Ѫ��
std::mutex mtx;

void playerAttack(int idx)  // idx: ��ұ��
{
	while (Blood > 0)
	{
		int injury = rand() % 20 + 1;
		{
			lock_guard<std::mutex> lock(mtx);  // ��������������ͱ��ͷ�
			if (Blood > 0)  // �����ж�
			{
				injury = rand() % 20 + 1;
				Blood -= injury;
				cout << "���" << idx << "�����" << injury << "���˺�, ";
				if (Blood <= 0)
				{
					cout << "��������boss" << endl;
				}
				else
				{
					cout << "��ǰbossѪ��Ϊ" << Blood << endl;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

int main()
{
	srand((unsigned)time(NULL));
	list<std::thread> li;
	for (int i = 1; i < 4; i++)
	{
		li.push_back(std::thread(playerAttack, i));
	}

	for (std::thread& t : li)
	{
		t.join();
	}

	//for (auto it = li.begin(); it != li.end(); it++)
	//{
	//	it->join();
	//}
	
	return 0;
}
#endif
// ###### �̼߳��ͬ��ͨ�ź�unique_lock ###############################################
#if 0
/*
	��˵����
	�̼߳�Ļ������ⳡ��������̷߳���һ�����ݣ��߳��д��ڷ��̴߳��ڶ����ݷ��̰߳�ȫ�Ĳ���
	�̼߳��ͬ��ͨ�ų����������������߳�����ֻ�������ˣ���������

	�ؼ�����
	std::condition_variable cv;  // �����������������̼߳��ͨ��
	cv.nofity_all();  // ֪ͨ���н���....
*/
#include<mutex>
#include<list>

/*  �򵥵�����˵��
	һ��ʼ��4�鼦�⣬�����߽�������������������Ȼ���ּ��⻹�У��ͷ������ȴ���
	�������߳�������ĳ4���̻߳��Ƚ��У���һ������������������м������һ�����뿪
	�������ڶ���������������ˣ����ĸ����������һ���֪ͨ���н��̣��ȴ��Ľ���������
	Ȼ����ʱֻ���������ڵȴ�����ǰ��ʣ��21�������������ߣ��������Ƕ����������ж�û��
	��������ȴ�״̬���������������������������⣬������21�������ߴӵȴ���Ϊ������
	��һ�������м��⣬������һ��....
*/

std::mutex mtx;
std::condition_variable cv;

int chicken = 4;

void cook()
{
	unique_lock<std::mutex> lock(mtx);  // ����
	cout << "��ΰ�޹�ϼ" << endl;
	while (chicken != 0)
	{
		cv.wait(lock);  // �������������Ϊ�㣬�͵ȼ��ⱻ���������ͷ������ȴ�״̬��
	}
	chicken += 5;  // ������鼦��
	cout << "������5�鼦�⣬��ǰ��������Ϊ: " << chicken << endl;
	cv.notify_all();
		
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void product()
{
	for (int i = 0; i < 4; i++)
	{
		cook();
	}
}

void pick(int idx)
{
	unique_lock<std::mutex> lock(mtx);  // ����
	cout << "�Ӳ��޾��ұ�������" << endl;
	while (chicken == 0)
	{
		cv.wait(lock);  // ���û�м��⣬�͵ȳ�������
	}
	chicken -= 1;  // ����һ�鼦��
	cout << "ѧ��" << idx << "������1�鼦�⣬��ǰ��������Ϊ: " << chicken << endl;
	if (chicken == 0)
	{
		cout << "siuuuuuuuuuuuuuuuuuuu" << endl;
		cv.notify_all();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main()
{	
	list<std::thread> tlist;
	std::thread t1(product);

	for (int i = 1; i < 25; i++)
	{
		tlist.push_back(std::thread(pick, i));
	}

	t1.join();
	for (std::thread& t : tlist)
	{
		t.join();
	}

	return 0;
}
#endif
// ###### ����CAS��ԭ��ģ�� ###########################################################
#if 0
/*
	�������ǱȽ��صģ��ر����ٽ������������������Ը��ӣ���
	ϵͳ���ۣ�CAS����֤++��--������ԭ�����Ծ��㹻��(��������)

	�����Ĳ��������ȡ��д�롢�޸ĵȣ��ڶ��̻߳�������ԭ�ӵģ�����Щ�����ǲ����жϵ�
	����һ��int��++��--������������ִ�У�ԭ�����ͱ�֤������������ִ������ִ��������
*/
#include<thread>

std::atomic_int count_ = 0;
// int count_ = 0;

void threadHandle()
{
	for (int i = 0; i < 1000; i++)
	{
		/*
			���ڱ���++�������Ƿ��̰߳�ȫ�ģ����ܵ����������̷ֱ߳�++�����ֻ����һ��
		*/
		count_++; 
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

int main()
{
	std::thread t1(threadHandle);
	std::thread t2(threadHandle);
	std::thread t3(threadHandle);

	t1.join();
	t2.join();
	t3.join();

	cout << count_ << endl;

	return 0;
}
#endif
// ###### ����ָ��shared_ptr ##########################################################
#if 0
#include<memory>
/*
	һ����ָ����Ҫ�ֶ���delete�ͷŵ�����������˻��߳������������׵����ڴ�й©
	����ָ��    ��֤��������Դ���Զ��ͷ�
	shared_ptr: ǿ����ָ�룬���Ըı���Դ�����ü���

	shared_ptr���н������õ����⣬�������weak_ptrһ��ʹ��
*/

class B;
class A {
public:
	A() { cout << "A()" << endl; }
	~A() { cout << "~A()" << endl; }
	// shared_ptr<B> _ptrb;
	weak_ptr<B> _ptrb;  // �������õĽ����������������ʹ��������ָ��
};
class B {
public:
	B() { cout << "B()" << endl; }
	~B() { cout << "~B()" << endl; }
	//shared_ptr<A> _ptra;
	weak_ptr<A> _ptra;
};

int main() {
	shared_ptr<A> pa(new A());
	shared_ptr<B> pb(new B());

	pa->_ptrb = pb;  // ��������
	pb->_ptra = pa;

	cout << pa.use_count() << endl;
	cout << pb.use_count() << endl;

	cout << pa->_ptrb.use_count() << endl;
	cout << pb->_ptra.use_count() << endl;

	return 0;
}
#endif
// ###### lambda���ʽ ####################################################################
#if 0
#include<functional>
#include<algorithm>
#include<vector>
int main()
{
	auto func = [](int a, int b)->int { return a + b; };
	vector<int> vec = { 1, 2, 3 ,4 ,5 ,6 };
	for_each(vec.begin(), vec.end(), [](int a)->void {cout << a << " "; });
	cout << endl;

}
#endif
// ###### ��������1 #################################################################
#if 0
#include<functional>
#include<thread>
class A
{
public:
	A(int a = 10) :_a(a) {};
	void show()
	{
		cout << "pdcHelloWorld" << endl;
	}
	
private:
	int _a;
};

int main()
{
	A a;
	// thread t1(a.show)  // ָ��󶨺�����ָ��ֻ�����ڵ��ú���
	// thread t1(A::show())  // �Ǿ�̬��Ա���ñ������ض��������
	thread t1(std::bind(&A::show, &a));
	thread t2(&A::show, &a);


	t1.join();
	t2.join();
	return 0;
}
#endif
// ###### ��������2 #################################################################
#if 0
/* 
	ͬ��ͨ��
*/
#include<thread>
#include<mutex>
#include<condition_variable>

std::mutex mtx;
std::condition_variable cv;

atomic_int cnt = 100;

void threadHandle01()
{
	for(;;)
	{
		unique_lock<mutex> lck(mtx);
		while (cnt > 0)
		{
			cout << "�߳�1����ȴ�״̬" << endl;
			cv.wait(lck);
			cout << "�߳�1�˳��ȴ�״̬" << endl;
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	}
}

void threadHandle02()
{
	for (;;)
	{
		{
			unique_lock<mutex> lck(mtx);
			cout << "�߳�2���: " << cnt-- << endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		cv.notify_all();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

int main()
{
	std::thread t2(threadHandle02); 
	std::thread t1(threadHandle01);

	/*
		t2��������t1���t2��ȡ�û�����Ȼ��������t2��ӡcnt=100��Ȼ��cnt��Ϊ99
		Ȼ��������ڣ������ͷţ���t2ִ��notify_all()������0.5s��ʱ���ڣ�t1�Ѿ�
		��ȡ���˻�������������Ȼ��t1����ȴ�״̬��ͨ��wait()�ͷ�������t2���õ�
		�˻�����Ȼ��������t2��ӡcnt=99��Ȼ��t2ִ��notify_all(),����t1�˳��ȴ�
		״̬������������
	*/
	
	t2.join();
	t1.join();
}
#endif
// ###### ��������3 #################################################################
#if 0
#include<thread>
#include<mutex>
#include<condition_variable>

std::mutex mtx;
std::condition_variable cv;
/*
	ͬ��ͨ��2
*/
void threadHandle01()
{
	// ÿ��һ�����һ��cv.notify_all();
	while (1)
	{
		cv.notify_all();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void threadHandle02()
{
	unique_lock<mutex> lck(mtx);
	while (1)
	{
		cout << "�߳�2����ȴ�״̬" << endl;
		cv.wait(lck);  // 
		cout << "�߳�2�˳��ȴ�״̬" << endl;
	}
}

int main()
{
	thread t2(threadHandle02);
	thread t1(threadHandle01);

	t2.join();
	t1.join();
}
#endif
// ###### ��������4 #################################################################
#if 0
/*
	�Բ�ͬ�̵߳Ļ���ִ�в�ͬ�Ĳ���
*/
#include<thread>
#include<condition_variable>
#include<mutex>

std::mutex mtx;
std::condition_variable cv;

std::atomic_bool label = false;

void threadHandle01()
{
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		label = true;
		cout << "�߳�1����֪ͨ" << endl;
		
		cv.notify_all();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		label = false;
	}

}

void threadHandle02()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		cout << "�߳�2����֪ͨ" << endl;
		
		cv.notify_all();
	}
}

int main()
{
	std::thread t1(threadHandle01);
	std::thread t2(threadHandle02);

	unique_lock<std::mutex> lck(mtx);
	while (1)
	{
		// ���������߳�ֻ�����߳�1��֪ͨ
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		/*
			�߳�1����֪ͨʱ�����label��Ϊtrue���߳���������labelȷʵ��
			true������������ִ�������������
			�߳�2����֪ͨʱ�����label��Ϊfalse���߳�����ʱ����label��false��
			���Ǽ���˯������ִ�������������
		*/
		cv.wait(lck, [] { return label == true; });
		cout << "���߳���Ӧ" << endl;
	}


	t1.join();
	t2.join();

}

#endif
// ###### ��������5 #################################################################
#if 1
class A
{
public:
	virtual void show()
	{
		cout << "pdcHelloWorld" << endl;
	}
};

class B : public A
{

};

int main()
{
	B b;
	b.show();
	return 0;
}
#endif