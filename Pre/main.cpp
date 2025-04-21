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
#if 0
#include<memory>
/*
	一般裸指针需要手动的delete释放掉。如果忘记了或者程序发生，就容易导致内存泄漏
	智能指针    保证能做到资源的自动释放
	shared_ptr: 强智能指针，可以改变资源的引用计数

	shared_ptr会有交叉引用的问题，可以配合weak_ptr一起使用
*/

class B;
class A {
public:
	A() { cout << "A()" << endl; }
	~A() { cout << "~A()" << endl; }
	// shared_ptr<B> _ptrb;
	weak_ptr<B> _ptrb;  // 交叉引用的解决方法：类内引用使用弱智能指针
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

	pa->_ptrb = pb;  // 交叉引用
	pb->_ptra = pa;

	cout << pa.use_count() << endl;
	cout << pb.use_count() << endl;

	cout << pa->_ptrb.use_count() << endl;
	cout << pb->_ptra.use_count() << endl;

	return 0;
}
#endif
// ###### lambda表达式 ####################################################################
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
// ###### 其他测试1 #################################################################
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
	// thread t1(a.show)  // 指向绑定函数的指针只能用于调用函数
	// thread t1(A::show())  // 非静态成员引用必须与特定对象相对
	thread t1(std::bind(&A::show, &a));
	thread t2(&A::show, &a);


	t1.join();
	t2.join();
	return 0;
}
#endif
// ###### 其他测试2 #################################################################
#if 0
/* 
	同步通信
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
			cout << "线程1进入等待状态" << endl;
			cv.wait(lck);
			cout << "线程1退出等待状态" << endl;
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
			cout << "线程2输出: " << cnt-- << endl;
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
		t2先启动，t1随后。t2先取得互斥锁然后上锁，t2打印cnt=100，然后cnt变为99
		然后出作用于，锁被释放，在t2执行notify_all()和休眠0.5s的时间内，t1已经
		先取得了互斥锁并上锁，然后t1进入等待状态，通过wait()释放锁，而t2又拿到
		了互斥锁然后上锁，t2打印cnt=99，然后t2执行notify_all(),于是t1退出等待
		状态并重新上锁。
	*/
	
	t2.join();
	t1.join();
}
#endif
// ###### 其他测试3 #################################################################
#if 0
#include<thread>
#include<mutex>
#include<condition_variable>

std::mutex mtx;
std::condition_variable cv;
/*
	同步通信2
*/
void threadHandle01()
{
	// 每隔一秒调用一次cv.notify_all();
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
		cout << "线程2进入等待状态" << endl;
		cv.wait(lck);  // 
		cout << "线程2退出等待状态" << endl;
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
// ###### 其他测试4 #################################################################
#if 0
/*
	对不同线程的唤醒执行不同的操作
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
		cout << "线程1发出通知" << endl;
		
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
		cout << "线程2发出通知" << endl;
		
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
		// 理论上主线程只接受线程1的通知
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		/*
			线程1发出通知时，会把label置为true，线程醒来发现label确实是
			true，于是上锁，执行下面的输出语句
			线程2发出通知时，会把label置为false，线程醒来时发现label是false，
			于是继续睡，不会执行下面的输出语句
		*/
		cv.wait(lck, [] { return label == true; });
		cout << "主线程响应" << endl;
	}


	t1.join();
	t2.join();

}

#endif
// ###### 其他测试5 #################################################################
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