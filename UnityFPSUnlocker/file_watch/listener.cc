#include "listener.hh"

#include <cerrno>
#include <cstring>
#include <linux/eventpoll.h>
#include <sys/inotify.h>

#include "dispatcher/epoller.hh"
#include "logger.hh"

namespace FileWatch {
    int Listener::inotify_fd_ = 0;
    absl::flat_hash_map<int, Listener::OnModified> Listener::registered_;

    Listener::Listener() {
        inotify_fd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (inotify_fd_ == -1) {
            error("[Listener::Register] [%d] %s", __LINE__, strerror(errno));
        }
    }

    int Listener::Register(const char* file_path, OnModified callback) {
        int wd = inotify_add_watch(inotify_fd_, file_path, IN_CLOSE_WRITE);
        if (wd == -1) {
            error("[Listener::Register] [%d] %s", __LINE__, strerror(errno));
            return -1;
        }

        auto res = EPoller::reserved_list_[0]->AddSocket(inotify_fd_, EPOLLIN);
        if (!res.ok()) {
            error("[Listener::Register] %s", res.message().data());
            return -1;
        }

        registered_[wd] = callback;
        logger("[Listener::Register] Registered: wd: %d %s", wd, file_path);
        return wd;
    }

    void Listener::OnReadable(uintptr_t s) {
        inotify_event event;
        int read_size = read(s, &event, sizeof(event));
        if (read_size == -1) {
            error("[Listener::OnReadable] [%d] %s", __LINE__, strerror(errno));
            return;
        }

        if (auto itor = registered_.find(event.wd); itor != registered_.end()) {
            itor->second(event.wd);
        }
    }
} // namespace FileWatch