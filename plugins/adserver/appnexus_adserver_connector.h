/** ad_server_connector_ex.cc                                 -*- C++ -*-
    Eric Robert, 03 April 2013
    Copyright (c) 2013 Datacratic.  All rights reserved.

    Example of a simple ad server connector.

*/

#include "soa/service/service_utils.h"
#include "soa/service/service_base.h"
#include "soa/service/json_endpoint.h"
#include "soa/service/zmq_named_pub_sub.h"
#include "rtbkit/plugins/adserver/http_adserver_connector.h"
#include "rtbkit/common/auction_events.h"

namespace RTBKIT {

/******************************************************************************/
/* AppNexus AD SERVER CONNECTOR                                                   */
/******************************************************************************/

struct AppNexusAdServerConnector : public HttpAdServerConnector
{
    AppNexusAdServerConnector(const std::string& serviceName,
                          std::shared_ptr<Datacratic::ServiceProxies> proxies)
        : HttpAdServerConnector(serviceName, proxies),
          publisher(getServices()->zmqContext) {
    }

    AppNexusAdServerConnector(Datacratic::ServiceProxyArguments & args,
                          const std::string& serviceName)
        : HttpAdServerConnector(serviceName, args.makeServiceProxies()),
          publisher(getServices()->zmqContext) {
    }

    AppNexusAdServerConnector(std::shared_ptr<ServiceProxies> const & proxies,
                          Json::Value const & json);

    void init(int port);
    void start();
    void shutdown();
    void handleNotificationRequests( const HttpHeader & header, const Json::Value & json, const std::string & jsonStr);

    /// Generic publishing endpoint to forward wins to anyone registered. Currently, there's only the
    /// router that connects to this.
    Datacratic::ZmqNamedPublisher publisher;
};

} // namepsace RTBKIT

