#ifndef EVENTLOOP_EVENTLOOP_HPP
#define EVENTLOOP_EVENTLOOP_HPP

#include <cstdint>

#include <unordered_map>
#include <queue>
#include <functional>
#include <utility>

#include <thread>
#include <mutex>
#include <condition_variable>

class EventLoop
{
public:
    EventLoop();
    virtual ~EventLoop();

    int64_t Start();
    void Stop(bool finishQueue = false);

    template <typename... Args>
    int64_t Register(int64_t aEventId, const std::function<void(Args...)>& aEventHandler)
    {
        int64_t result = static_cast<int64_t>(0);
        EventHandlerBase* eventHandler = new EventHandler<Args...>(aEventHandler);
        if(eventHandler != nullptr)
        {
            auto eventHandlerMapIter = mEventHandlerMap.find(aEventId);
            if(eventHandlerMapIter == mEventHandlerMap.end())
            {
                mEventHandlerMap[aEventId] = eventHandler;
            }
        }
        else
        {
            result = static_cast<int64_t>(-2);
        }

        return result;
    }

    template <typename... Args>
    int64_t Post(int64_t aEventId, Args&&... aArgs) 
    {
        if(mThreadStopRequest || mThreadFinishQueueRequest)
        {
            return static_cast<int64_t>(-1);
        }

        int64_t result = static_cast<int64_t>(0);

        auto eventHandlerMapIter = mEventHandlerMap.find(aEventId);
        if(eventHandlerMapIter != mEventHandlerMap.end())
        {
            mEventQueueMutex.lock();

            EventBase* event = new Event<Args...>(aEventId, eventHandlerMapIter->second, std::forward<Args>(aArgs)...);
            if(event != nullptr)
            {
                mEventQueue.push(event);
                mEventQueueCondition.notify_one();
            }
            else
            {
                result = static_cast<int64_t>(-2);
            }

            mEventQueueMutex.unlock();
        }
        else
        {
            result = static_cast<int64_t>(-3);
        }

        return result;
    }

    void ClearAll();

private:
    class EventHandlerBase
    {
    public:
        EventHandlerBase() {}
        virtual ~EventHandlerBase() {}
    };

    template <typename... Args>
    class EventHandler : public EventHandlerBase
    {
    public:
        const std::function<void(Args...)> mFunc;

    public:
        EventHandler(const std::function<void(Args...)>& aFunc)
            : EventHandlerBase(),
            mFunc(aFunc) {}
        virtual ~EventHandler() {}
    };

    class EventBase
    {
    public:
        int64_t mEventId;
        EventHandlerBase* mEventHandler;

    public:
        EventBase(int64_t aEventId, EventHandlerBase* aEventHandler)
            : mEventId(aEventId),
            mEventHandler(aEventHandler) {}
        virtual ~EventBase() {}

        virtual void doEvent() {}
    };

    template <typename... Args>
    class Event : public EventBase 
    {
    public:
        Event(int64_t aEventId, EventHandlerBase* aEventHandler, Args&&... aArgs)
            : EventBase(aEventId, aEventHandler),
            mEventFunc(std::bind(((EventHandler<Args...>*)aEventHandler)->mFunc, 
                std::forward<Args>(aArgs)...)) {}

        virtual ~Event() {}

        virtual void doEvent()
        {
            mEventFunc();
        }

    public:
        std::function<void()> mEventFunc;
    };

    virtual void MainLoop();

    std::unordered_map<int64_t, EventHandlerBase*> mEventHandlerMap;
    std::queue<EventBase*> mEventQueue;
    std::mutex mEventQueueMutex, mEventLoopMutex;
    std::condition_variable mEventQueueCondition;

    std::thread* mThread;
    bool mThreadStopRequest;
    bool mThreadFinishQueueRequest;
};

#endif // EVENTLOOP_EVENTLOOP_HPP