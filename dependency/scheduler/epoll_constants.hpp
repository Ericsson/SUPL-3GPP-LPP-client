#pragma once
#include <cstdint>
#include <sys/epoll.h>

static CONSTEXPR uint32_t EPOLL_IN    = EPOLLIN;
static CONSTEXPR uint32_t EPOLL_OUT   = EPOLLOUT;
static CONSTEXPR uint32_t EPOLL_ERR   = EPOLLERR;
static CONSTEXPR uint32_t EPOLL_HUP   = EPOLLHUP;
static CONSTEXPR uint32_t EPOLL_RDHUP = EPOLLRDHUP;
