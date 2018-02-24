#include <iostream>

#include <EventLoop.hpp>

using namespace std;

EventLoop::EventLoop()
    : mThread(nullptr), mThreadStopRequest(true),
    mThreadFinishQueueRequest(true)
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
    mThreadFinishQueueRequest = false;

    mThread = new thread(bind(&EventLoop::MainLoop, this));
    if(mThread == nullptr)
    {
        return static_cast<int64_t>(-2);
    }

    return static_cast<int64_t>(0);
}

void EventLoop::Stop(bool finishQueue) 
{
    if(finishQueue)
    {
        mThreadFinishQueueRequest = true;
    }
    else
    {
        mThreadStopRequest = true;
    }

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

    mThreadStopRequest = true;
}

void EventLoop::ClearAll()
{
    unique_lock<mutex> scopedLock(mEventQueueMutex);
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
        unique_lock<mutex> spinLock(mEventLoopMutex);
        if(mEventQueue.empty())
        {
            if(mThreadFinishQueueRequest) break;
            mEventQueueCondition.wait(spinLock);
        }
            
        if(mThreadStopRequest) break;

        unique_lock<mutex> scopedLock(mEventQueueMutex);
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
