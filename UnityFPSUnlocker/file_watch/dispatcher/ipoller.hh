#ifndef IPOLLER_HEADER
#define IPOLLER_HEADER

#include <absl/status/status.h>

#include "ibusiness_event.hh"

class IPoller {
public:
    IPoller(){};
    virtual ~IPoller(){};
    virtual absl::Status AddSocket(intptr_t s, long eventflags) = 0;
    virtual absl::Status ModSocket(intptr_t s, long eventflags) = 0;
    virtual absl::Status RemoveSocket(intptr_t s) = 0;
    // while (IsRunning) {
    virtual void Poll() = 0;
    // }
    void SetBusiness(IBusinessEvent* op) { op_ = op; }

protected:
    IBusinessEvent* op_ = nullptr;
};

#endif // ipoller.hh