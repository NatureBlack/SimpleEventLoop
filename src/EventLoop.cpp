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

    mThreadStopRequest = true;
}

void EventLoop::ClearAll()
{
    lock_guard<mutex> scopedLock(mEventQueueMutex);
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
        // scopedLock for event queue
        {
            unique_lock<mutex> scopedLock(mEventQueueMutex);
            if(mEventQueue.empty())
            {
                if(mThreadFinishQueueRequest) break;
                mEventQueueCondition.wait(scopedLock, [this]{ return !mEventQueue.empty(); });
            }

            if(!mEventQueue.empty())
            {
                event = mEventQueue.front();
                mEventQueue.pop();
            }
        }

        if(event != nullptr)
        {
            event->doEvent();
            delete event;
            event = nullptr;
        }
    }
}
