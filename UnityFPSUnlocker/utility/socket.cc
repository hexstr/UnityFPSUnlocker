#include <endian.h>
#include <fcntl.h>

#include <logger.hh>
#include <socket.hh>
#include <unistd.h>

using namespace std;

ssize_t xsendmsg(int sockfd, const struct msghdr* msg, int flags) {
    int sent = sendmsg(sockfd, msg, flags);
    if (sent < 0) {
        logger("xsendmsg error: %s", strerror(errno));
    }
    return sent;
}

ssize_t xrecvmsg(int sockfd, struct msghdr* msg, int flags) {
    int rec = recvmsg(sockfd, msg, flags);
    if (rec < 0) {
        logger("xrecvmsg error: %s", strerror(errno));
    }
    return rec;
}

ssize_t xxread(int fd, void* buf, size_t count) {
    size_t read_sz = 0;
    ssize_t ret;
    do {
        ret = read(fd, (std::byte*)buf + read_sz, count - read_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            logger("xxread error: %s", strerror(errno));
            return ret;
        }
        read_sz += ret;
    } while (read_sz != count && ret != 0);
    if (read_sz != count) {
        logger("xxread (%zu != %zu)", count, read_sz);
    }
    return read_sz;
}

ssize_t xwrite(int fd, const void* buf, size_t count) {
    size_t write_sz = 0;
    ssize_t ret;
    do {
        ret = write(fd, (std::byte*)buf + write_sz, count - write_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            logger("xwrite error: %s", strerror(errno));
            return ret;
        }
        write_sz += ret;
    } while (write_sz != count && ret != 0);
    if (write_sz != count) {
        logger("xwrite (%zu != %zu)", count, write_sz);
    }
    return write_sz;
}

static size_t socket_len(sockaddr_un* sun) {
    if (sun->sun_path[0])
        return sizeof(sa_family_t) + strlen(sun->sun_path) + 1;
    else
        return sizeof(sa_family_t) + strlen(sun->sun_path + 1) + 1;
}

socklen_t setup_sockaddr(sockaddr_un* sun, const char* name) {
    memset(sun, 0, sizeof(*sun));
    sun->sun_family = AF_UNIX;
    strcpy(sun->sun_path + 1, name);
    return socket_len(sun);
}

bool get_client_cred(int fd, sock_cred* cred) {
    socklen_t len = sizeof(ucred);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, cred, &len) != 0)
        return false;
    char buf[4096];
    len = sizeof(buf);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERSEC, buf, &len) != 0)
        return false;
    buf[len] = '\0';
    cred->context = buf;
    return true;
}

static int send_fds(int sockfd, void* cmsgbuf, size_t bufsz, const int* fds, int cnt) {
    iovec iov = {
        .iov_base = &cnt,
        .iov_len = sizeof(cnt),
    };
    msghdr msg = {
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };

    if (cnt) {
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = bufsz;
        cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof(int) * cnt);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        memcpy(CMSG_DATA(cmsg), fds, sizeof(int) * cnt);
    }

    return xsendmsg(sockfd, &msg, 0);
}

int send_fds(int sockfd, const int* fds, int cnt) {
    if (cnt == 0) {
        return send_fds(sockfd, nullptr, 0, nullptr, 0);
    }
    vector<char> cmsgbuf;
    cmsgbuf.resize(CMSG_SPACE(sizeof(int) * cnt));
    return send_fds(sockfd, cmsgbuf.data(), cmsgbuf.size(), fds, cnt);
}

int send_fd(int sockfd, int fd) {
    if (fd < 0) {
        return send_fds(sockfd, nullptr, 0, nullptr, 0);
    }
    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    return send_fds(sockfd, cmsgbuf, sizeof(cmsgbuf), &fd, 1);
}

static void* recv_fds(int sockfd, char* cmsgbuf, size_t bufsz, int cnt) {
    iovec iov = {
        .iov_base = &cnt,
        .iov_len = sizeof(cnt),
    };
    msghdr msg = {
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = cmsgbuf,
        .msg_controllen = bufsz
    };

    xrecvmsg(sockfd, &msg, MSG_WAITALL);
    cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);

    if (msg.msg_controllen != bufsz ||
        cmsg == nullptr ||
        cmsg->cmsg_len != CMSG_LEN(sizeof(int) * cnt) ||
        cmsg->cmsg_level != SOL_SOCKET ||
        cmsg->cmsg_type != SCM_RIGHTS) {
        return nullptr;
    }

    return CMSG_DATA(cmsg);
}

vector<int> recv_fds(int sockfd) {
    vector<int> results;

    // Peek fd count to allocate proper buffer
    int cnt;
    recv(sockfd, &cnt, sizeof(cnt), MSG_PEEK);
    if (cnt == 0)
        return results;

    vector<char> cmsgbuf;
    cmsgbuf.resize(CMSG_SPACE(sizeof(int) * cnt));

    void* data = recv_fds(sockfd, cmsgbuf.data(), cmsgbuf.size(), cnt);
    if (data == nullptr)
        return results;

    results.resize(cnt);
    memcpy(results.data(), data, sizeof(int) * cnt);

    return results;
}

int recv_fd(int sockfd) {
    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    void* data = recv_fds(sockfd, cmsgbuf, sizeof(cmsgbuf), 1);
    if (data == nullptr)
        return -1;

    int result;
    memcpy(&result, data, sizeof(int));
    return result;
}

int read_int(int fd) {
    int val;
    if (xxread(fd, &val, sizeof(val)) != sizeof(val))
        return -1;
    return val;
}

int read_int_be(int fd) {
    uint32_t val;
    if (xxread(fd, &val, sizeof(val)) != sizeof(val))
        return -1;
    return ntohl(val);
}

void write_int(int fd, int val) {
    if (fd < 0)
        return;
    xwrite(fd, &val, sizeof(val));
}

void write_int_be(int fd, int val) {
    uint32_t nl = htonl(val);
    xwrite(fd, &nl, sizeof(nl));
}

void read_string(int fd, std::string& str) {
    int len = read_int(fd);
    str.clear();
    str.resize(len);
    xxread(fd, str.data(), len);
}

string read_string(int fd) {
    string str;
    read_string(fd, str);
    return str;
}

void write_string(int fd, string_view str) {
    if (fd < 0)
        return;
    write_int(fd, str.size());
    xwrite(fd, str.data(), str.size());
}
