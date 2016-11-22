// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../net/utils.hpp"

#include <deque>
#include <cstring>

#include <event2/util.h>

namespace mk {
namespace net {

int storage_init(sockaddr_storage *storage, socklen_t *salen,
                 const char *family, const char *address, const char *port,
                 Var<Logger> logger) {
    int _family;
    /* TODO: support also AF_INET, AF_INET6, ... */
    if (strcmp(family, "PF_INET") == 0) {
        _family = PF_INET;
    } else if (strcmp(family, "PF_INET6") == 0) {
        _family = PF_INET6;
    } else {
        warn("utils:storage_init: invalid family");
        return -1;
    }
    return storage_init(storage, salen, _family, address, port, logger);
}

int storage_init(sockaddr_storage *storage, socklen_t *salen, int _family,
                 const char *address, const char *port, Var<Logger> logger) {
    const char *errstr;
    int _port = (int)mkp_strtonum(port, 0, 65535, &errstr);
    if (errstr != nullptr) {
        warn("utils:storage_init: invalid port");
        return -1;
    }
    return storage_init(storage, salen, _family, address, _port, logger);
}

int storage_init(sockaddr_storage *storage, socklen_t *salen, int _family,
                 const char *address, int _port, Var<Logger> logger) {
    int result;

    if (_port < 0 || _port > 65535) {
        logger->warn("utils:storage_init: invalid port");
        return -1;
    }

    memset(storage, 0, sizeof(*storage));
    switch (_family) {

    case PF_INET6: {
        sockaddr_in6 *sin6 = (sockaddr_in6 *)storage;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = htons(_port);
        if (address != nullptr) {
            result = inet_pton(AF_INET6, address, &sin6->sin6_addr);
            if (result != 1) {
                logger->warn("utils:storage_init: invalid addr");
                return -1;
            }
        } else {
            sin6->sin6_addr = in6addr_any;
        }
        *salen = sizeof(sockaddr_in6);
        break;
    }

    case PF_INET: {
        sockaddr_in *sin = (sockaddr_in *)storage;
        sin->sin_family = AF_INET;
        sin->sin_port = htons(_port);
        if (address != nullptr) {
            result = inet_pton(AF_INET, address, &sin->sin_addr);
            if (result != 1) {
                logger->warn("utils:storage_init: invalid addr");
                return -1;
            }
        } else {
            sin->sin_addr.s_addr = INADDR_ANY;
        }
        *salen = sizeof(sockaddr_in);
        break;
    }

    default:
        logger->warn("utils:storage_init - invalid family");
        return -1;
    }

    return 0;
}

socket_t socket_create(int domain, int type, int protocol, Var<Logger> logger) {
    socket_t filedesc;
    int result;

    filedesc = socket(domain, type, protocol);
    if (filedesc == -1) {
        logger->warn("utils:socket: cannot create socket");
        return -1;
    }

    result = evutil_make_socket_nonblocking(filedesc);
    if (result != 0) {
        logger->warn("utils:socket: cannot make nonblocking");
        (void)evutil_closesocket(filedesc);
        return -1;
    }

    return filedesc;
}

std::string unreverse_ipv6(std::string s) {
    size_t i = 0, added = 0;
    std::deque<char> r;
    for (; i < s.size(); ++i) {
        if ((i % 2) == 0) {
            if (!isxdigit(s[i])) {
                break;
            }
            r.push_front(s[i]);
            if ((++added % 4) == 0 && added <= 28) {
                r.push_front(':');
            }
        } else {
            if (s[i] != '.') {
                return "";
            }
        }
    }
    if (s.substr(i) != "ip6.arpa" && s.substr(i) != "ip6.arpa.") {
        return "";
    }
    return std::string(r.begin(), r.end());
}

std::string unreverse_ipv4(std::string s) {
    std::deque<char> r, t;
    size_t i = 0, seen = 0;
    unsigned cur = 0;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (cur > 255) {
                return "";
            }
            if (seen++ > 0) {
                r.push_front('.');
            }
            r.insert(r.begin(), t.begin(), t.end());
            t.clear();
            cur = 0;
        } else if (isdigit(s[i])) {
            t.push_back(s[i]);
            char tmpstr[] = {s[i], '\0'};
            cur = cur * 10 + atoi(tmpstr);
        } else {
            break;
        }
    }
    if (s.substr(i) != "in-addr.arpa" && s.substr(i) != "in-addr.arpa.") {
        return "";
    }
    return std::string(r.begin(), r.end());
}

} // namespace net
} // namespace mk