#include "listener.hh"

#include <cerrno>
#include <cstring>
#include <linux/eventpoll.h>
#include <linux/inotify.h>
#include <sys/inotify.h>

#include "dispatcher/epoller.hh"
#include "utility/logger.hh"

namespace FileWatch {
    Listener::Listener() {
        inotify_fd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (inotify_fd_ == -1) {
            ERROR("[Listener::Register] [%d] %s", __LINE__, strerror(errno));
        }
    }

    int Listener::Register(const char* file_path, OnModified callback) {
        int wd = inotify_add_watch(inotify_fd_, file_path, IN_CLOSE_WRITE | IN_MODIFY | IN_DELETE_SELF);
        if (wd == -1) {
            ERROR("[Listener::Register] [%d] %s", __LINE__, strerror(errno));
            return -1;
        }

        auto res = EPoller::reserved_list_[0]->AddSocket(inotify_fd_, EPOLLIN);
        if (!res.ok()) {
            ERROR("[Listener::Register] %s", res.message().data());
            return -1;
        }

        registered_[wd] = callback;
        LOG("[Listener::Register] Registered: wd: %d %s", wd, file_path);
        return wd;
    }

    void Listener::OnReadable(uintptr_t s) {
        inotify_event event;
        int read_size = read(s, &event, sizeof(event));
        if (read_size == -1) {
            ERROR("[Listener::OnReadable] [%d] %s", __LINE__, strerror(errno));
            return;
        }
        /*
                if (event.mask & IN_CREATE) {
                    LOGI("The file was created.");
                }
                else if (event.mask & IN_DELETE) {
                    LOGI("The file was deleted.");
                }
                else if (event.mask & IN_MODIFY) {
                    LOGI("The file was modified.");
                }
        */
        if (event.mask & IN_CLOSE_WRITE) {
            if (auto itor = registered_.find(event.wd); itor != registered_.end()) {
                itor->second(event.wd);
            }
        }
        else if (event.mask & IN_DELETE_SELF) {
            ERROR("File was deleted");
        }
    }
} // namespace FileWatch