/* 预备知识点
	1. 单例模式
	2. queue队列容器
	3. c++11多线程编程，线程互斥，线程同步通信和unique_lock
	4. 基于CAS的原子模型
	5. 智能指针shared_ptr
	6. lambda表达式
	7. 生产者-消费者线程模型
	8. MySql数据库编程
*/

#include<iostream>
using namespace std;

// ###### 单例模式 #############################################################
#if 0
class SingleInstance
{
public:
	// 获取类的唯一实例对象的接口方法
	static SingleInstance* getInstacne()
	{
		/*
			静态的局部变量，程序的启动阶段，内存就存在于数据端上
			但是初始化是等到运行到该代码时才初始化的
			静态局部变量只执行一次初始化
			函数静态局部变量的初始化，在汇编指令上已经自动添加线程互斥指令了
		*/
		static SingleInstance instance;
		return& instance;
	}

	int a;
	int b;

private:
	SingleInstance()  // 构造函数私有化
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
		static int a = value;  // 初始化
		return a;
	}
	int returnValueB(int value)
	{
		b = value;  // 赋值
		return b;
	}

	static int b;
};
int A::b = 1;  // 类内声明，类外初始化

void test1()
{
	A a;
	for (int i = 0; i < 10; ++i)
	{
		// 静态成员变量a，通过returnValue多次初始化，但输出的结果都是第一次的0
		cout << a.returnValueA(i) << " ";
	}cout << endl;

	for (int i = 0; i < 10; ++i)
	{
		// 这是赋值，不是初始化，因此b的值会随着改变
		cout << a.returnValueB(i) << " ";
	}cout << endl;
}

int main()
{
	// 单例模式
	test1();

	// 这个函数必须写成静态函数，才能用类作用域去调用
	SingleInstance* s1 = SingleInstance::getInstacne();
	SingleInstance* s2 = SingleInstance::getInstacne();

	cout << "通过s1访问a：" << s1->a << endl;
	cout << "通过s1访问b：" << s1->b << endl;
	cout << "通过s2访问a：" << s2->a << endl;
	cout << "通过s2访问b：" << s2->b << endl;
	
	return 0;
}
#endif

// ###### queue队列操作 ######################################################
#if 0
#include<queue>
/*  首先：queue是一个容器适配器
	适配器底层没有自己的数据结构，它是另外一个容器的封装，它的方法，
	全部由底层依赖的容器实现的，没有实现自己的迭代器。
	push入队，pop出队，front查看队头元素，back查看队尾元素，empty判空，size元素个数
	依赖于deque
*/

int main()
{
	// queue<int> que = { 1, 2, 3, 4 ,5 };
	queue<int> que;
	for (int i = 0; i < 5; i++)
	{
		que.push(i + 1);
	}
	// for(auto item: que) 没有迭代器，无法正常遍历
	while (!que.empty())
	{
		cout << que.front() << endl;
		que.pop();
	}

	return 0;
}
#endif
// ###### c++11多线程操作 ######################################################
#if 0
#include<thread>  // 不要省略std::

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
	std::thread t1(threadHandle01);  // 线程对象传入一个线程函数
	std::thread t2(threadHandle02);
	/*
		一般情况下，要求主线程在子线程结束之后结束，否则会报错
		可以有如下操作实现：
	*/
	t1.join();  // 主线程执行到此处，等待子线程t1执行完成再接下去执行
	t2.detach();  // 与主线程分离，主线程不需要管他(主线程跑完了它还没跑完没事)

	// 一些线程中的操作
	std::this_thread::sleep_for(std::chrono::seconds(2));  // 休眠两秒

	return 0;
}
#endif
// ###### 线程间的互斥操作 ####################################################
#if 0
#include<thread>
#include<list>
#include<mutex>
/*
	代码中可能存在一些线程不安全操作，比如在多个对全局变量Blood的改变(Blood--)
	Blood--在汇编中包含3个操作。可能出现：比如Blood是20，当线程一进入执行--还未执行
	完成时，此时线程二已经进入，Blood还是20，于是两个线程分别执行--的结果却是19。

	关键操作，lock_guard<std::mutex> lock(mtx);
	取到这把锁的进程进行上锁，把其他线程堵塞在外，等到解锁后(出了锁的作用域)
	一般是锁+双重判断
*/

int Blood = 1000;  // boss的血量
std::mutex mtx;

void playerAttack(int idx)  // idx: 玩家编号
{
	while (Blood > 0)
	{
		int injury = rand() % 20 + 1;
		{
			lock_guard<std::mutex> lock(mtx);  // 锁，出了作用域就被释放
			if (Blood > 0)  // 二次判定
			{
				injury = rand() % 20 + 1;
				Blood -= injury;
				cout << "玩家" << idx << "造成了" << injury << "点伤害, ";
				if (Blood <= 0)
				{
					cout << "并击败了boss" << endl;
				}
				else
				{
					cout << "当前boss血量为" << Blood << endl;
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
// ###### 线程间的同步通信和unique_lock ###############################################
#if 0
/*
	简单说明：
	线程间的互斥问题场景：多个线程访问一个数据，线程中存在非线程存在对数据非线程安全的操作
	线程间的同步通信场景：生产者消费者场景，只有生产了，才能消费

	关键操作
	std::condition_variable cv;  // 定义条件变量，做线程间的通信
	cv.nofity_all();  // 通知所有进程....
*/
#include<mutex>
#include<list>

/*  简单的流程说明
	一开始有4块鸡肉，生产者进程先启动，先上锁，然后发现鸡肉还有，就放锁，等待，
	消费者线程启动，某4个线程会先进行，第一个进入后上锁，发现有鸡肉就拿一个，离开
	解锁，第二个第三个亦是如此，第四个拿走了最后一块后，通知所有进程，等待的进入阻塞，
	然而此时只有生产者在等待。但前面剩下21个消费者阻塞者，于是他们都依次上锁判断没有
	鸡肉后进入等待状态，解锁。随后生产者生产五个鸡肉，把所有21个消费者从等待变为阻塞，
	第一个发现有鸡肉，就拿走一个....
*/

std::mutex mtx;
std::condition_variable cv;

int chicken = 4;

void cook()
{
	unique_lock<std::mutex> lock(mtx);  // 上锁
	cout << "阿伟罗桂霞" << endl;
	while (chicken != 0)
	{
		cv.wait(lock);  // 如果鸡肉数量不为零，就等鸡肉被抢完再煮【释放锁，等待状态】
	}
	chicken += 5;  // 生产五块鸡肉
	cout << "生产了5块鸡肉，当前鸡肉数量为: " << chicken << endl;
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
	unique_lock<std::mutex> lock(mtx);  // 上锁
	cout << "朴昌罗精忠报国助大韩" << endl;
	while (chicken == 0)
	{
		cv.wait(lock);  // 如果没有鸡肉，就等厨子做完
	}
	chicken -= 1;  // 消费一块鸡肉
	cout << "学生" << idx << "消费了1块鸡肉，当前鸡肉数量为: " << chicken << endl;
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
// ###### 基于CAS的原子模型 ###########################################################
#if 0
/*
	互斥锁是比较重的，特别是临界区代码做的事情稍稍复杂，多
	系统理论：CAS来保证++，--操作的原子特性就足够了(无锁操作)

	其对象的操作（如读取、写入、修改等）在多线程环境中是原子的，即这些操作是不可中断的
	比如一般int的++，--有三条汇编代码执行，原子类型保证这三条汇编必须执行完再执行其他的
*/
#include<thread>

std::atomic_int count_ = 0;
// int count_ = 0;

void threadHandle()
{
	for (int i = 0; i < 1000; i++)
	{
		/*
			由于本身++操作就是非线程安全的，可能导致如两个线程分别++后可能只加了一个
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
// ###### 智能指针shared_ptr ##########################################################
#if 1

int main()
{


	return 0;
}
#endif