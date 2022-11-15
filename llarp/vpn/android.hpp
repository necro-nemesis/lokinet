#pragma once

#include <stdio.h>
#include <unistd.h>

#include "platform.hpp"
#include "common.hpp"
#include <llarp.hpp>

namespace llarp::vpn
{
  class AndroidInterface : public NetworkInterface
  {
    const int m_fd;

   public:
    AndroidInterface(InterfaceInfo info, int fd) : NetworkInterface{std::move(info)}, m_fd{fd}
    {
      if (m_fd == -1)
        throw std::runtime_error(
            "Error opening AndroidVPN layer FD: " + std::string{strerror(errno)});
    }

    virtual ~AndroidInterface()
    {
      if (m_fd != -1)
        ::close(m_fd);
    }

    int
    PollFD() const override
    {
      return m_fd;
    }

    net::IPPacket
    ReadNextPacket() override
    {
      std::vector<byte_t> pkt;
      pkt.reserve(net::IPPacket::MaxSize);
      const auto n = read(m_fd, pkt.data(), pkt.capacity());
      pkt.resize(std::min(std::max(ssize_t{}, n), static_cast<ssize_t>(pkt.capacity())));
      return net::IPPacket{std::move(pkt)};
    }

    bool
    WritePacket(net::IPPacket pkt) override
    {
      const auto sz = write(m_fd, pkt.data(), pkt.size());
      if (sz <= 0)
        return false;
      return sz == static_cast<ssize_t>(pkt.size());
    }
  };

  class AndroidRouteManager : public IRouteManager
  {
    void AddRoute(net::ipaddr_t, net::ipaddr_t) override{};

    void DelRoute(net::ipaddr_t, net::ipaddr_t) override{};

    void
    AddDefaultRouteViaInterface(NetworkInterface&) override{};

    void
    DelDefaultRouteViaInterface(NetworkInterface&) override{};

    void
    AddRouteViaInterface(NetworkInterface&, IPRange) override{};

    void
    DelRouteViaInterface(NetworkInterface&, IPRange) override{};

    std::vector<net::ipaddr_t>
    GetGatewaysNotOnInterface(NetworkInterface&) override
    {
      return std::vector<net::ipaddr_t>{};
    };
  };

  class AndroidPlatform : public Platform
  {
    const int fd;
    AndroidRouteManager _routeManager{};

   public:
    AndroidPlatform(llarp::Context* ctx) : fd{ctx->androidFD}
    {}

    std::shared_ptr<NetworkInterface>
    ObtainInterface(InterfaceInfo info, AbstractRouter*) override
    {
      return std::make_shared<AndroidInterface>(std::move(info), fd);
    }
    IRouteManager&
    RouteManager() override
    {
      return _routeManager;
    }
  };

}  // namespace llarp::vpn
