#pragma once

#include <chrono>
#include "i_rc_lookup_handler.hpp"

#include <llarp/util/thread/threading.hpp>

#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>

struct llarp_dht_context;

namespace llarp
{
  class NodeDB;
  class EventLoop;

  namespace service
  {
    struct Context;

  }  // namespace service

  struct ILinkManager;

  struct RCLookupHandler final : public I_RCLookupHandler
  {
   public:
    using Work_t = std::function<void(void)>;
    using WorkerFunc_t = std::function<void(Work_t)>;
    using CallbacksQueue = std::list<RCRequestCallback>;

    ~RCLookupHandler() override = default;

    void
    AddValidRouter(const RouterID& router) override EXCLUDES(_mutex);

    void
    RemoveValidRouter(const RouterID& router) override EXCLUDES(_mutex);

    void
    SetRouterWhitelist(
        const std::vector<RouterID>& whitelist,
        const std::vector<RouterID>& greylist,
        const std::vector<RouterID>& greenlist

        ) override EXCLUDES(_mutex);

    bool
    HaveReceivedWhitelist() const override;

    void
    GetRC(const RouterID& router, RCRequestCallback callback, bool forceLookup = false) override
        EXCLUDES(_mutex);

    bool
    PathIsAllowed(const RouterID& remote) const override EXCLUDES(_mutex);

    bool
    SessionIsAllowed(const RouterID& remote) const override EXCLUDES(_mutex);

    bool
    IsGreylisted(const RouterID& remote) const override EXCLUDES(_mutex);

    // "greenlist" = new routers (i.e. "green") that aren't fully funded yet
    bool
    IsGreenlisted(const RouterID& remote) const override EXCLUDES(_mutex);

    // registered just means that there is at least an operator stake, but doesn't require the node
    // be fully funded, active, or not decommed.  (In other words: it is any of the white, grey, or
    // green list).
    bool
    IsRegistered(const RouterID& remote) const override EXCLUDES(_mutex);

    bool
    CheckRC(const RouterContact& rc) const override;

    bool
    GetRandomWhitelistRouter(RouterID& router) const override EXCLUDES(_mutex);

    bool
    CheckRenegotiateValid(RouterContact newrc, RouterContact oldrc) override;

    void
    PeriodicUpdate(llarp_time_t now) override;

    void
    ExploreNetwork() override;

    size_t
    NumberOfStrictConnectRouters() const override;

    void
    Init(
        llarp_dht_context* dht,
        std::shared_ptr<NodeDB> nodedb,
        std::shared_ptr<EventLoop> loop,
        WorkerFunc_t dowork,
        ILinkManager* linkManager,
        service::Context* hiddenServiceContext,
        const std::unordered_set<RouterID>& strictConnectPubkeys,
        const std::set<RouterContact>& bootstrapRCList,
        bool useWhitelist_arg,
        bool isServiceNode_arg);

    std::unordered_set<RouterID>
    Whitelist() const
    {
      util::Lock lock{_mutex};
      return whitelistRouters;
    }

   private:
    void
    HandleDHTLookupResult(RouterID remote, const std::vector<RouterContact>& results);

    bool
    HavePendingLookup(RouterID remote) const EXCLUDES(_mutex);

    bool
    RemoteInBootstrap(const RouterID& remote) const;

    void
    FinalizeRequest(const RouterID& router, const RouterContact* const rc, RCRequestResult result)
        EXCLUDES(_mutex);

    mutable util::Mutex _mutex;  // protects pendingCallbacks, whitelistRouters

    llarp_dht_context* _dht = nullptr;
    std::shared_ptr<NodeDB> _nodedb;
    std::shared_ptr<EventLoop> _loop;
    WorkerFunc_t _work = nullptr;
    service::Context* _hiddenServiceContext = nullptr;
    ILinkManager* _linkManager = nullptr;

    /// explicit whitelist of routers we will connect to directly (not for
    /// service nodes)
    std::unordered_set<RouterID> _strictConnectPubkeys;

    std::set<RouterContact> _bootstrapRCList;
    std::unordered_set<RouterID> _bootstrapRouterIDList;

    std::unordered_map<RouterID, CallbacksQueue> pendingCallbacks GUARDED_BY(_mutex);

    bool useWhitelist = false;
    bool isServiceNode = false;

    // whitelist = active routers
    std::unordered_set<RouterID> whitelistRouters GUARDED_BY(_mutex);
    // greylist = fully funded, but decommissioned routers
    std::unordered_set<RouterID> greylistRouters GUARDED_BY(_mutex);
    // greenlist = registered but not fully-staked routers
    std::unordered_set<RouterID> greenlistRouters GUARDED_BY(_mutex);

    using TimePoint = std::chrono::steady_clock::time_point;
    std::unordered_map<RouterID, TimePoint> _routerLookupTimes;
  };

}  // namespace llarp
