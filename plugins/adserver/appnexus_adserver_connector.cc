/** appnexus_adserver_connector.cc                                 -*- C++ -*-
    Eric Robert, 03 April 2013
    Copyright (c) 2013 Datacratic.  All rights reserved.

    Example of a simple ad server connector.

*/

#include "appnexus_adserver_connector.h"


using namespace RTBKIT;

AppNexusAdServerConnector::
AppNexusAdServerConnector(std::shared_ptr<ServiceProxies> const & proxies, Json::Value const & json) :
    HttpAdServerConnector(json.get("name", "appnexus-adserver").asString(), proxies),
    publisher(getServices()->zmqContext) {
}

void AppNexusAdServerConnector::init(int port) {
    auto services = getServices();

    // Initialize our base class
    HttpAdServerConnector::init(services->config);

    auto onNotificationRequest = [=] (const HttpHeader & header,
                        const Json::Value & json,
                        const std::string & jsonStr) {
        this->handleNotificationRequests(header, json, jsonStr);
    };
    registerEndpoint(port, onNotificationRequest);

    // Publish the endpoint now that it exists.
    HttpAdServerConnector::bindTcp();
    
    // And initialize the generic publisher on a predefined range of ports to try avoiding that
    // collision between different kind of service occurs.
    //publisher.init(services->config, serviceName() + "/logger");
    //publisher.bindTcp(services->ports->getRange("adServer.logger"));
}

void AppNexusAdServerConnector::start() {
    recordLevel(1.0, "up");
    HttpAdServerConnector::start();
    publisher.start();
}


void AppNexusAdServerConnector::shutdown() {
    HttpAdServerConnector::shutdown();
    publisher.shutdown();
}


void AppNexusAdServerConnector::handleNotificationRequests(
		const HttpHeader & header,
		const Json::Value & json, 
		const std::string & jsonStr) {
	
	AppNexus::NotifyRequestRoot notifyRoot;
    Datacratic::DefaultDescription<AppNexus::NotifyRequestRoot> desc;
    desc.parseJson(&notifyRoot, jsonContext);

	auto& notifyRequest = notifyRoot.requests.front();

	for(auto& tag : notifyRequest.tags)
		publishWin(tag.auctionId64, tag.creativeId, tag.pricePaid, notifyRequest.timestamp, Json::parse(tag.customNotifyData), notifyRequest.userId64, "", notifyRequest.timestamp); 
}

namespace {

struct AtInit {
    AtInit()
    {
        AdServerConnector::registerFactory("AppNexus", [](std::shared_ptr<ServiceProxies> const & proxies,
                                                      Json::Value const & json) {
            auto server = new AppNexusAdServerConnector(proxies, json);

            int port = json.get("port", "12401").asInt();
            server->init(port);
            return server;
        });
    }
} atInit;

}

