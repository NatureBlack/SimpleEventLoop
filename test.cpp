#include <iostream>
#include <cstdio>
#include <string>
#include <chrono>
#include <functional>

#include "EventLoop.hpp"

using namespace std;

typedef enum 
{
    EVENT_ID_0,
    EVENT_ID_1,
    EVENT_ID_2,
    EVENT_ID_3,
    EVENT_ID_4,
} EVENT_ID;

void Event0Func()
{
    fprintf(stdout, "L%d::%s\n", __LINE__, __FUNCTION__);
}

void Event1Func(double value) 
{
    fprintf(stdout, "L%d::%s - %lf\n", __LINE__, __FUNCTION__, value);
}

void Event2Func(std::string str, int value) 
{
    fprintf(stdout, "L%d::%s - %s %d\n", __LINE__, __FUNCTION__, str.c_str(), value);
}

void Event3Func(EventLoop* el)
{
    fprintf(stdout, "L%d::%s\n", __LINE__, __FUNCTION__);
    el->Post(EVENT_ID_3, el);
}

void Event4Func(EventLoop* el)
{
    fprintf(stdout, "L%d::%s\n", __LINE__, __FUNCTION__);
    el->Post(EVENT_ID_4, el);
}


class test
{
public:
    test()
    {
        this->rr = 666;
        cout << this << endl;
    }

    int rr;
};

void testRef(test t1, const test& t2, test&& t3)
{
    cout << "add t1: " << &t1 << endl;
    cout << "add t2: " << &t2 << endl;
    cout << "add t3: " << &t3 << endl;
}

int main(int argc, char** argv) {
    cout << "BEGIN" << endl;

    testRef(test(), test(), test());

    EventLoop gEventLoop;

    function<void()> gEvent0Func = Event0Func;
    function<void(double)> gEvent1Func = bind(Event1Func, placeholders::_1);
    function<void(string, int)> gEvent2Func = bind(Event2Func, placeholders::_1, placeholders::_2);
    gEventLoop.Register(EVENT_ID_0, gEvent0Func);
    gEventLoop.Register(EVENT_ID_1, gEvent1Func);
    gEventLoop.Register(EVENT_ID_2, gEvent2Func);
    gEventLoop.Register(EVENT_ID_3, function<void(EventLoop*)>(Event3Func));
    gEventLoop.Register(EVENT_ID_4, function<void(EventLoop*)>(Event4Func));


    gEventLoop.Start();

    gEventLoop.Post(EVENT_ID_2, std::string("Hello World!"), 1234);
    gEventLoop.Post(EVENT_ID_0);
    gEventLoop.Post(EVENT_ID_1, 42.31);
    gEventLoop.Post(EVENT_ID_2, std::string("Hello World Again!"), 12345);
    gEventLoop.Post(EVENT_ID_1, 452.361);
    gEventLoop.Post(EVENT_ID_0);
    gEventLoop.Post(EVENT_ID_3, &gEventLoop);
    gEventLoop.Post(EVENT_ID_4, &gEventLoop);


    while(true);

    gEventLoop.Stop();

    cout << "DONE." << endl;

    return 0;
}
