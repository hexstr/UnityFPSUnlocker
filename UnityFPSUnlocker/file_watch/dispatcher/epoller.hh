#ifndef EPOLLER_HEADER
#define EPOLLER_HEADER

#include <cstdint>
#ifdef __linux__

#include <sys/epoll.h>

#include <array>
#include <vector>

#include "ipoller.hh"

constexpr int MAX_EVENT_NUMBER = 64;

class EPoller : public IPoller {
public:
    EPoller(IBusinessEvent* business);
    ~EPoller(){};

    absl::Status AddSocket(intptr_t s, long eventflags) override;
    absl::Status ModSocket(intptr_t s, long eventflags) override;
    absl::Status RemoveSocket(intptr_t s) override;

    void Poll() override;
    void HandleEvents(intptr_t s, uint32_t event);

    std::array<epoll_event, MAX_EVENT_NUMBER> event_array_{};

    static int SetNonBlocking(int);
    static std::vector<IPoller*> reserved_list_; // SubReactor list

protected:
    int epoller_inst_ = 0;
};

#endif // #ifdef __linux__

#endif // epoller.hh