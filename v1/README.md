# 云游戏服务器连接池

## 1. 关键技术点
MySQL数据库编程、单例模式、queue队列容器、C++11多线程编程、线程互斥、线程同步通信和unique_lock、基于CAS的原子整形、智能指针shared_ptr、lambda表达式、生产者-消费者线程模型。
<br>


## 2. 连接池功能点介绍

连接池一般包含了数据库连接所用的ip地址、port端口号、用户名和密码以及其它的性能参数，例如初始连接量，最大连接量，最大空闲时间、连接超时时间等，该项目是基于C++语言实现的连接池，主要也是实现以上几个所有连接池都支持的通用基础功能。

**初始连接量（initSize）**：表示连接池事先会和MySQL Server创建initSize个数的connection连接，当应用发起MySQL访问时，不用再创建和MySQL Server新的连接，直接从连接池中获取一个可用的连接就可以，使用完成后，并不去释放connection，而是把当前connection再归还到连接池当中。

**最大连接量（maxSize）**：当并发访问MySQL Server的请求增多时，初始连接量已经不够使用了，此时会根据新的请求数量去创建更多的连接给应用去使用，但是新创建的连接数量上限是maxSize，不能无限制的创建连接，因为每一个连接都会占用一个socket资源，一般连接池和服务器程序是部署在一台主机上的，如果连接池占用过多的socket资源，那么服务器就不能接收太多的客户端请求了。当这些连接使用完成后，再次归还到连接池当中来维护。

**最大空闲时间（maxIdleTime）**：当访问MySQL的并发请求多了以后，连接池里面的连接数量会动态增加，上限是maxSize个，当这些连接用完再次归还到连接池当中。如果在指定的maxIdleTime里面，这些新增加的连接都没有被再次使用过，那么新增加的这些连接资源就要被回收掉，只需要保持初始连
接量initSize个连接就可以了。

**~~连接超时时间（connectionTimeout）~~**

## 3. 功能实现设计 

新的功能点：用户分为普通用户和vip用户，普通用户和vip用户共享初始连接量的连接。当并发量较大时，申请量超过空闲连接时，__即超过初始连接量的连接只提供给vip用户__。会额外创建新的连接供vip用户申请，总的连接不超过最大连接量。当没有空闲的连接供对应类型的用户时，会进入排队等待状态，轮到后可以正常申请连接。

(可选项): 排队时，vip用户无论顺序都排在普通用户前面，可以优先申请连接。

连接池主要包含了以下功能点： 

- 1.连接池只需要一个实例，所以ConnectionPool以单例模式进行设计。 

- 2.从ConnectionPool中可以获取和MySQL的连接Connection。

- 3.空闲连接Connection全部维护在一个线程安全的Connection队列中，使用线程互斥锁保证队列的线程安全。 

- 4.如果Connection队列为空，还需要再获取连接，此时需要动态创建连接，上限数量是maxSize。 

- 5.队列中空闲连接时间超过maxIdleTime的就要被释放掉，只保留初始的initSize个连接就可以了，这个功能点肯定需要放在独立的线程中去做。 

- 6.如果Connection队列为空，而此时连接的数量已达上限maxSize，那么等待connectionTimeout时间，如果还获取不到空闲的连接，那么获取连接失败，此处从Connection队列获取空闲连接，可以使用带超时时间的mutex互斥锁来实现连接超时时间。 

- 7.用户获取的连接用shared_ptr智能指针来管理，用lambda表达式定制连接释放的功能（不真正释放连接，而是把连接归还到连接池中）。 

- 8.连接的生产和连接的消费采用生产者-消费者线程模型来设计，使用了线程间的同步通信机制条件变量和互斥锁。


## 4. 主要功能点的分析

- **阶段一：**
    
    连接池_connectQueue中的连接加上被申请走的连接数量等于initSize, 如果此时_connectQueue中还有空闲连接，那么无论是普通用户还是vip用户都可以直接申请里面的连接。如果_connectQueue中的连接空了，就进入第二阶段。

- **阶段二：**

    此时总的连接数小于maxSize，普通用户想再申请连接，就要**排队**，而vip用户想要再申请连接，可以让生产者生产额外的连接，但是总的数量不能超过maxSize。当总的数量大于等于maxSize时，就进入第三阶段。

- **阶段三：**

    此时总的连接数大于等于maxSize，即使是vip用户，也得进行排队。

- **排队机制：**

    在ConnectionPool中设置两个双端队列deque: 
    ```cpp
    deque<CommonUser*> commonUserDeque;
    deque<VipUser*> vipUserDeque;
    ```
    双端队列支持：

    push_back()——模拟入队<br>
    pop_front()——模拟出队<br>
    erase(iterator it)——通过遍历确定迭代器然后删除，模拟用户退出排队<br>

    ### 用户进入排队等待状态

    比起之前连接池的项目，不再有连接池获取连接的超时时间_connectionTimeout。

    - 对于普通用户，如果判断_connectQueue为空，则将该用户对应的指针放入commonUserDeque中兵进入等待状态，接受cv.notify_all()的唤醒并判断一下条件：
        - _connectQueue是否不为空？
        - vipUserDeque是否为空？即没有vip用户在排队
        - commonUserDeque.front()是不是自己？
        
        若都满足，则让该用户申请走连接

        **注：由于生产者新生产的连接只供给vip用户使用，故这里申请走的连接只能是用户主动析构或用户超时未处理回收到_connectQueue中的连接。**

    - 对于vip用户，如果判断_connectQueue为空，则先确定——总的连接数是否大于等于maxSize，如果没有，则按照连接池项目的申请方法正常申请。如果超过了，就得像普通用户一样排队，但vip用户依然有优先的排队通道：
        - _connectQueue是否不为空？
        - vipUserDeque.front()是不是自己？

    ### 当总连接数小于maxSize,大于initSize时:
    - ConnectionPool::getConnecion()中:
        ```cpp
        _produceForVip = true;  
		cv.wait(lock, [&]() -> bool {
					return _designedForVip == true;
				}
			);
		_designedForVip = false;
        ```
        先把_produceForVip置为true,只有为true的时候，生产者线程才能被唤醒。然后进入等待状态。

    - ConnectionPool::produceConnectionTask()中:
        ```cpp
        cv.wait(lock, [&]() -> bool {
				return _produceForVip == true;
			}
		);
        ```
        ```cpp
        _produceForVip = false;
		_designedForVip = true;

		cv.notify_all(); 
        ```