#include <iostream>

#include <EventLoop.hpp>

using namespace std;

EventLoop::EventLoop()
    : mThread(nullptr), mThreadStopRequest(true)
{
}

EventLoop::~EventLoop()
{
    Stop();
}

int64_t EventLoop::Start()
{
    if(mThread != nullptr)
    {
        return static_cast<int64_t>(-2);
    }

    mThreadStopRequest = false;

    mThread = new thread(std::bind(&EventLoop::MainLoop, this));
    if(mThread == nullptr)
    {
        return static_cast<int64_t>(-2);
    }

    return static_cast<int64_t>(0);
}

void EventLoop::Stop() 
{
    mThreadStopRequest = true;

    mEventQueueCondition.notify_one();

    if(mThread != nullptr) 
    {
        if(mThread->joinable()) 
        {
            mThread->join();
        }

        delete mThread;
        mThread = nullptr;
    }

    ClearAll();
    for(auto eventHandlerMapIter = mEventHandlerMap.begin();
        eventHandlerMapIter != mEventHandlerMap.end();
        ++eventHandlerMapIter) 
    {
        delete static_cast<EventHandlerBase*>(eventHandlerMapIter->second);
    }
    mEventHandlerMap.clear();
}

void EventLoop::ClearAll()
{
    std::unique_lock<std::mutex> scopedLock(mEventQueueMutex);
    while(!mEventQueue.empty())
    {
        EventBase* eventIter = mEventQueue.front();
        mEventQueue.pop();
        delete eventIter;
    }
}

void EventLoop::MainLoop()
{
    EventBase* event = nullptr;

    while(!mThreadStopRequest)
    {
        std::unique_lock<std::mutex> scopedLock(mEventQueueMutex);
        if(mEventQueue.empty())
        {
            mEventQueueCondition.wait(scopedLock);
        }
            
        if(mThreadStopRequest) break;

        if(!mEventQueue.empty())
        {
            event = mEventQueue.front();
            mEventQueue.pop();
        }
        scopedLock.unlock();

        if(event != nullptr)
        {
            event->doEvent();
            delete event;
            event = nullptr;
        }
    }
}
