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

    vector<thread> _vecThread;

// ###### 单独测试vip用户 ##################################################################
#if 0

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

#endif
// ###### 单独测试普通用户 #################################################################
#if 0

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
// ###### 普通用户和vip用户 ###############################################################
#if 1
    srand((unsigned)time(NULL));
    vector<int> _vecUserType;

    for (int i = 0; i < 60; ++i)
    {
        // 0表示普通用户，1表示vip用户
        int _userType = rand() % 2;
        _vecUserType.push_back(_userType);
        if (_userType == 1)
        {
            _vecVipUser.push_back(shared_ptr<VipUser>(new VipUser));
        }
        else
        {
            _vecCommonUser.push_back(shared_ptr<CommonUser>(new CommonUser));
        }
    }

    auto _itVecVipUser = _vecVipUser.begin();
    auto _itVecCommonUser = _vecCommonUser.begin();

    for (int& userType : _vecUserType)
    {
        if (userType == 1)
        {
            _vecThread.push_back(std::thread(
                [&]() -> void {
                    (*_itVecVipUser++)->toConnect(_connectPool);
                }
            )
            );
        }
        else
        {
            _vecThread.push_back(std::thread(
                [&]() -> void {
                    (*_itVecCommonUser++)->toConnect(_connectPool);
                }
            )
            );
        }
    }

    for (thread& t : _vecThread)
    {
        t.join();
    }

#endif
    std::this_thread::sleep_for(std::chrono::seconds(100));

    return 0;
}
