#ifdef __linux__

#include <fcntl.h>

#include <memory>

#include "epoller.hh"
#include "logger.hh"

std::vector<IPoller*> EPoller::reserved_list_;

EPoller::EPoller(IBusinessEvent* business) {
    epoller_inst_ = epoll_create1(0);
    op_ = business;
    op_->poller_ = reinterpret_cast<IPoller*>(this);
}

int EPoller::SetNonBlocking(int fd) {
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    return fcntl(fd, F_SETFL, new_opt);
}

absl::Status EPoller::AddSocket(intptr_t s, long eventflags) {
    epoll_event _event;
    _event.data.u64 = s;
    _event.events = eventflags;
    if (epoll_ctl(epoller_inst_, EPOLL_CTL_ADD, s, &_event) == -1) {
        return absl::InternalError(strerror(errno));
    }
    return absl::OkStatus();
}

absl::Status EPoller::ModSocket(intptr_t s, long eventflags) {
    epoll_event _event;
    _event.data.fd = s;
    _event.events = eventflags;
    if (epoll_ctl(epoller_inst_, EPOLL_CTL_MOD, s, &_event) == -1) {
        return absl::InternalError(strerror(errno));
    }
    return absl::OkStatus();
}

absl::Status EPoller::RemoveSocket(intptr_t s) {
    if (epoll_ctl(epoller_inst_, EPOLL_CTL_DEL, s, nullptr) == -1) {
        return absl::InternalError(strerror(errno));
    } else {
        return absl::OkStatus();
    }
}

void EPoller::Poll() {
    int CompEventNum = epoll_wait(epoller_inst_, &event_array_[0], MAX_EVENT_NUMBER, 3000);
    for (int i = 0; i < CompEventNum; ++i) {
        HandleEvents(reinterpret_cast<intptr_t>(event_array_[i].data.ptr), event_array_[i].events);
    }
}

void EPoller::HandleEvents(intptr_t s, uint32_t event) {
    if (event & EPOLLRDHUP) {
        op_->OnClose(s);
    }

    else if (event & EPOLLIN) {
        op_->OnReadable(s);
    }

    else if (event & EPOLLOUT) {
        op_->OnWritable(s);
    }

    else if (event & EPOLLERR) {
        op_->OnError(s);
    }
}

#endif // #ifdef __linux__