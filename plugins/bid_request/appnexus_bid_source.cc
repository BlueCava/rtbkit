/** appnexus_bid_source.cc                                 -*- C++ -*-
    Eric Robert, 14 Aug 2013
    Copyright (c) 2013 Datacratic.  All rights reserved.

*/

#include "appnexus.h"
#include "appnexus_bid_request.h"
#include "appnexus_parsing.h"
#include "appnexus_bid_source.h"
#include "soa/service/http_header.h"

using namespace Datacratic;
using namespace RTBKIT;

BidRequest AppnexusBidSource::generateRandomBidRequest() {

	AppNexus::BidRequestRoot bidRequestRoot;

	auto &bidRequest = bidRequestRoot.bidRequest;
	bidRequest.timestamp = "2011-06-07 14:39:35";
	bidRequest.bidderTimeoutMs = 100;
	bidRequest.allowExclusive = false;
	bidRequest.debugRequested = false;
	bidRequest.test = true;
	bidRequest.singlePhase = true;

	bidRequest.members.emplace_back();
	auto &member = bidRequest.members.back();
	member.id = Id(6);
	bidRequest.members.emplace_back();
	member = bidRequest.members.back();
	member.id = Id(12);

	bidRequest.tags.emplace_back();
	auto &tag = bidRequest.tags.back();
	tag.id = Id(13);
	tag.sizes.push_back("728x90");
	tag.position.val = AppNexus::AdPosition::UNKNOWN;
	tag.inventorySourceId = Id(476);
	tag.auctionId64 = 8984480746668973511;
	tag.tagFormat = "iframe";
    tag.reservePrice = 1.000; 
    tag.adProfileId = 382;
    tag.inventoryAudits.emplace_back();
	auto &tagInventoryAudit =  tag.inventoryAudits.back();
    tagInventoryAudit.auditorMemberId = Id(17);
    tagInventoryAudit.inventoryAttributes.push_back(1431);
	tagInventoryAudit.inventoryAttributes.push_back(1432);
	tagInventoryAudit.inventoryAttributes.push_back(1887);
    tagInventoryAudit.contentCategories.push_back(171);

	auto &bidInfo = bidRequest.bidInfo;
	bidInfo.userId64 = rng.random();
	bidInfo.userAgent = "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.5; en-US;rv:1.9.0.3) Gecko/2008092414 Firefox/3.0.3";
	bidInfo.noFlash = false;
	bidInfo.noCookies = false;
	bidInfo.ipAddress = "96.246.152.18";
	bidInfo.acceptedLanguages = "en-US,en;q=0.8";
	bidInfo.country = "US";
	bidInfo.region = "NY";
	bidInfo.city = "New York";
	bidInfo.postalCode = "10014";
	bidInfo.dma = 501;
	bidInfo.timeZone = "America/New_York";
	bidInfo.url = "http://www.foodandwine.com/recipes/";
	bidInfo.domain = "foodandwine.com";
	bidInfo.sellingMemberId = Id(3);
	bidInfo.inventoryClass = "class_2";
	bidInfo.withinIframe = false;
	bidInfo.segments.emplace_back();
	auto &segment = bidInfo.segments.back();
	segment.id = Id(103);
	segment.memberId = Id(6);
	segment.code = "PotentialCarBuyer";
	segment.lastSeenMin = 21108407;
	bidInfo.inventoryAudits.emplace_back();
	auto &inventoryAudit = bidInfo.inventoryAudits.back();
	inventoryAudit.auditorMemberId = Id();
	inventoryAudit.intendedAudience = "young_adult";
	inventoryAudit.inventoryAttributes.push_back(2);
	inventoryAudit.inventoryAttributes.push_back(3);
	inventoryAudit.inventoryAttributes.push_back(6);
	inventoryAudit.inventoryAttributes.push_back(7);
	inventoryAudit.contentCategories.push_back(39);

    StructuredJsonPrintingContext context;
    DefaultDescription<AppNexus::BidRequestRoot> desc;
    desc.printJson(&bidRequestRoot, context);
    std::string content = context.output.toString();
	int length = content.length();

    std::string httpRequest = ML::format(
        "%s %s HTTP/1.1\r\n"
        "Content-Length: %zd\r\n"
        "Content-Type: application/json\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n"
        "%s",
        verb,
        path,
        length,
        content.c_str());

    write(httpRequest);
	std::unique_ptr<BidRequest> br(fromAppNexus(bidRequestRoot.bidRequest, "appnexus", "appnexus"));
    return BidRequest(*br);
}

auto
AppnexusBidSource::
parseResponse(const std::string& rawResponse) -> std::pair<bool, std::vector<Bid>>
{
	AppNexus::BidResponseRoot responseRoot;
    if(rawResponse.empty()) {
        return std::make_pair(false, std::vector<Bid>());
    }

    try {
        HttpHeader header;
        header.parse(rawResponse);
        if (!header.contentLength || header.resource != "200") {
            //std::cerr << rawResponse << std::endl;
            return std::make_pair(false, std::vector<Bid>());
        }

        ML::Parse_Context context("payload", header.knownData.c_str(), header.knownData.size());
        StreamingJsonParsingContext json(context);
        DefaultDescription<AppNexus::BidResponseRoot> desc;
        desc.parseJson(&responseRoot, json);
    }
    catch (const std::exception & exc) {
        std::cerr << "invalid response received: " << exc.what() << std::endl;
        return std::make_pair(false, std::vector<Bid>());
    }

    std::vector<Bid> bids;

    for(auto i = 0; i != responseRoot.responses.size(); ++i) {
        Bid bid;
        bid.adSpotId = Id(responseRoot.responses[i].auctionId.val);
        bid.maxPrice = responseRoot.responses[i].price.val;
        bids.push_back(bid);
    }

    return std::make_pair(true, bids);
}

namespace {

struct AtInit {
    AtInit()
    {
        BidSource::registerBidSourceFactory("appnexus", [](Json::Value const & json) {
            return new AppnexusBidSource(json);
        });
    }
} atInit;

}

