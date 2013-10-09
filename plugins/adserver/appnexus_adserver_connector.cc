/** appnexus_adserver_connector.cc                                 -*- C++ -*-
    Eric Robert, 03 April 2013
    Copyright (c) 2013 Datacratic.  All rights reserved.

    Example of a simple ad server connector.

*/

#include "appnexus_adserver_connector.h"
#include "../bid_request/appnexus.h"
#include "../bid_request/appnexus_parsing.h"
#include "rtbkit/common/json_holder.h"


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

	StructuredJsonParsingContext jsonContext(json);
	AppNexus::NotifyRequestRoot notifyRoot;
	DefaultDescription<AppNexus::NotifyRequestRoot> desc;
	desc.parseJson(&notifyRoot, jsonContext);

	auto& notifyRequest = notifyRoot.notifyRequest;

	UserIds userIds;
	userIds.add(Id(notifyRequest.userId64.val), ID_EXCHANGE);

	Date timestamp = Date::parse_date_time(notifyRequest.timestamp, "%y-%m-%d", "%H:%M:%S");	
	
	for(auto& tag : notifyRequest.tags) {
		if(tag.notifyType == "won" ||
		   tag.notifyType == "kept") {			
			auto customData = Json::parse(tag.customNotifyData);
			AccountKey accountKey(customData["accountId"].asString());			

			publishWin(Id(tag.auctionId64.val),
					   Id(tag.creativeId.val),
					   USD_CPM(tag.pricePaid.val),
					   timestamp,
					   customData,
					   userIds,
					   accountKey,
					   timestamp);
		}
	}
}

namespace {

struct AtInit {
    AtInit()
    {
        AdServerConnector::registerFactory("appnexus", [](std::shared_ptr<ServiceProxies> const & proxies,
                                                      Json::Value const & json) {
            auto server = new AppNexusAdServerConnector(proxies, json);

            int port = json.get("port", "12401").asInt();
            server->init(port);
            return server;
        });
    }
} atInit;

}

