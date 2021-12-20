#pragma once

#include <stdio.h>
#include <unistd.h>

#include <llarp/ev/vpn.hpp>
#include "common.hpp"
#include <llarp.hpp>

namespace llarp::vpn
{
  class AndroidInterface : public NetworkInterface
  {
    const int m_fd;
    const InterfaceInfo m_Info;  // likely 100% ignored on android, at least for now

   public:
    AndroidInterface(InterfaceInfo info, int fd) : m_fd(fd), m_Info(info)
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
      net::IPPacket pkt{};
      const auto sz = read(m_fd, pkt.buf, sizeof(pkt.buf));
      if (sz >= 0)
        pkt.sz = std::min(sz, ssize_t{sizeof(pkt.buf)});
      return pkt;
    }

    bool
    WritePacket(net::IPPacket pkt) override
    {
      const auto sz = write(m_fd, pkt.buf, pkt.sz);
      if (sz <= 0)
        return false;
      return sz == static_cast<ssize_t>(pkt.sz);
    }

    std::string
    IfName() const override
    {
      return m_Info.ifname;
    }
  };

  class AndroidRouteManager : public IRouteManager
  {
    void AddRoute(IPVariant_t, IPVariant_t) override{};

    void DelRoute(IPVariant_t, IPVariant_t) override{};

    void AddDefaultRouteViaInterface(std::string) override{};

    void DelDefaultRouteViaInterface(std::string) override{};

    void
    AddRouteViaInterface(NetworkInterface&, IPRange) override{};

    void
    DelRouteViaInterface(NetworkInterface&, IPRange) override{};

    std::vector<IPVariant_t> GetGatewaysNotOnInterface(std::string) override
    {
      return std::vector<IPVariant_t>{};
    };
  };

  class AndroidPlatform : public Platform
  {
    const int fd;
    AndroidRouteManager _routeManager{};

   public:
    AndroidPlatform(llarp::Context* ctx) : fd(ctx->androidFD)
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
