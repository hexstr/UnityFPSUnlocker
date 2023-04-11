#include "listener.hh"

#include <cerrno>
#include <cstring>
#include <linux/eventpoll.h>
#include <linux/inotify.h>
#include <sys/inotify.h>
#include <tuple>

#include "dispatcher/epoller.hh"
#include "utility/logger.hh"

namespace FileWatch {
    Listener::Listener() {
        inotify_fd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (inotify_fd_ == -1) {
            ERROR("[Listener::Register] [%d] %s", __LINE__, strerror(errno));
        }
    }

    int Listener::Register(const char* file_path, OnModified mod_callback, OnDeleted del_callback) {
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

        registered_[wd] = std::make_tuple(mod_callback, del_callback);
        LOG("[Listener::Register] Registered: wd: %d %s", wd, file_path);
        return wd;
    }

    void Listener::Remove(int wd) {
        inotify_rm_watch(inotify_fd_, wd);
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
                else*/
        if (event.mask & IN_DELETE) {
            LOG("The file was deleted.");
            if (auto itor = registered_.find(event.wd); itor != registered_.end()) {
                auto del_callback = std::get<1>(itor->second);
                if (del_callback != nullptr) {
                    del_callback();
                }
            }
        }
        /*
                else if (event.mask & IN_MODIFY) {
                    LOGI("The file was modified.");
                }
        */
        else if (event.mask & IN_CLOSE_WRITE) {
            if (auto itor = registered_.find(event.wd); itor != registered_.end()) {
                auto mod_callback = std::get<0>(itor->second);
                if (mod_callback != nullptr) {
                    mod_callback(event.wd);
                }
            }
        }
        else if (event.mask & IN_DELETE_SELF) {
            ERROR("File was deleted");
        }
    }
} // namespace FileWatch