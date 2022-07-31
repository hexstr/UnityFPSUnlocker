#ifndef IBUSINESS_EVENT_HEADER
#define IBUSINESS_EVENT_HEADER

#include <cstdint>

class IPoller;

class IBusinessEvent {
public:
    virtual ~IBusinessEvent() {}
    virtual void OnAcceptable(uintptr_t){};
    virtual void OnClose(uintptr_t){};
    virtual void OnReadable(uintptr_t){};
    virtual void OnWritable(uintptr_t){};
    virtual void OnError(uintptr_t){};
    IPoller* poller_ = nullptr;
};

#endif // ibusiness_event.hh