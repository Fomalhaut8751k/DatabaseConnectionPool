#include<iostream>
#include<vector>

#include"public.h"
#include"Connection.h"
#include"ConnectionPool.h"
#include"User.h"

using namespace std;

int main()
{
    ConnectionPool* _connectPool = ConnectionPool::getConnectionPool();
    vector<shared_ptr<CommonUser>> _vecCommonUser;
    vector<shared_ptr<VipUser>> _vecVipUser;

#if 1
    vector<thread> _vecThread;

    for (int i = 0; i < 50; ++i)
    {
        //_vecCommonUser.push_back(shared_ptr<CommonUser>(new CommonUser));
        _vecVipUser.push_back(shared_ptr<VipUser>(new VipUser));
    }
    
    for (shared_ptr<VipUser>& _pUser: _vecVipUser)
    {
        _vecThread.push_back(std::thread([&]()->void {
                _pUser->toConnect(_connectPool);
            }
        ));
    }

    for (thread& t : _vecThread)
    {
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(100));

#endif
#if 0
    vector<thread> _vecThread;

    for (int i = 0; i < 30; ++i)
    {
        _vecCommonUser.push_back(shared_ptr<CommonUser>(new CommonUser));
    }

    for (shared_ptr<CommonUser>& _pUser : _vecCommonUser)
    {
        _vecThread.push_back(std::thread([&]()->void {
            _pUser->toConnect(_connectPool);
            }
        ));
    }

    for (thread& t : _vecThread)
    {
        t.join();
    }
#endif

    return 0;
}
