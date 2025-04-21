# c++数据库连接池

- 预备知识点
    ## 单例模式(懒汉)

    在整个项目中，连接池ConnectionPool有且仅有一个实例，并通过成员函数返回这个实例的地址。
<br>
<br>

    ## queue队列容器

    在连接池ConnectionPool，所有空闲连接将被放置在一个队列Queue中
<br>
<br>
    ## 生产者-消费者线程模型

    线程之间的同步通信，涉及到等待(wait)和通知(notify)。消费者消费物品之前，要保证物品的数量大于等于1，如果没有便**通知**生产者生产。生产者生产物品后，**通知**消费者前来消费....

    在该项目中，该线程模型的场景：当并发量较大，用户发起申请却没有空闲连接时，就需要一个独立的线程去生产额外的空闲连接，当然总的数量不能超过maxSize

    条件变量: 
    ```std::condition_variable cv```

    ```wait()```, ```notify_all()```的使用：

    **注意：cv.wait(lck)释放锁进入等待状态，在其他进程的cv.notify_all()唤醒后，<u>会重新上锁</u>。**

    在该项目的一种情况：

    - produceConnectionTask()上锁，发现_connectQueue不是空的，就释放锁，进入等待状态。
    - 用户进程通过调用getConnection()来申请空闲连接的使用，上锁，因为_connectQueue不为空，所以直接申请得到，出作用域释放锁。
    - 当并发量过大，把所有空闲连接都申请完了，此时_connectQueue为空，因此下一个。。。
<br>
<br>

    ## bind绑定器

    由于成员函数的调用依赖于对象，需要把调用需要的this指针通过绑定器绑定上。

    ```thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));```

    对于thread的构造，对于成员函数的构造:

    ```std::thread t(&ClassName::MemberFunction, objectPointer, args...);```

    - &ClassName::MemberFunction 是成员函数的指针
    - objectPointer 是指向类实例的指针或引用，指明成员函数在哪个对象上执行。
    - args... 是传递给成员函数的参数。

    因此其实也可以不用bind
    
    ```thread produce(&ConnectionPool::produceConnectionTask, this);```
<br>
<br>
    ## 智能指针shared_ptr

    智能指针在出了作用域后会自动析构，可以帮助我们更有效的管理数据。在该项目中，getConnection()返回的连接的类型就是模板类型为Connection的shared_ptr, 同时，我们希望连接的回收不是将其析构掉而是归还给连接池，这里就可以通过<u>自定义删除器</u>来实现：

    ```
    shared_ptr<Connection> sp(_connectQueue.front(),
		[&](Connection* pcon){  
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();
			_connectQueue.push(pcon);
		}
	);
    ```
<br>
<br>
    ## 基于CAS的原子模型

    
<br>
<br>

    >c++11多线程编程，线程互斥
<br>
<br>



    >lambda表达式

<br>
<br>
    >MySql数据库编程

