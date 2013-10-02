/** appnexus_win_source.cc                                 -*- C++ -*-
    Eric Robert, 20 Aug 2013
    Copyright (c) 2013 Datacratic.  All rights reserved.

*/

#include "appnexus_win_source.h"
#include "rtbkit/core/post_auction/post_auction_loop.h"

using namespace RTBKIT;

AppNexusWinSource::
AppNexusWinSource(NetworkAddress address) :
    WinSource(std::move(address)) {
}


AppNexusWinSource::
AppNexusWinSource(Json::Value const & json) :
    WinSource(json) {
}

void
AppNexusWinSource::
sendWin(const BidRequest& bidRequest, const Bid& bid, const Amount& winPrice)
{
    PostAuctionEvent event;
    event.type = PAE_WIN;
    event.auctionId = bidRequest.auctionId;
    event.adSpotId = bid.adSpotId;
    event.timestamp = Date::now();
    event.winPrice = winPrice;
    event.uids = bidRequest.userIds;
    event.account = bid.account;
    event.bidTimestamp = bid.bidTimestamp;

    sendEvent(event);
}


void
AppNexusWinSource::
sendImpression(const BidRequest& bidRequest, const Bid& bid)
{
    PostAuctionEvent event;
    event.type = PAE_CAMPAIGN_EVENT;
    event.label = "IMPRESSION";
    event.auctionId = bidRequest.auctionId;
    event.adSpotId = bid.adSpotId;
    event.timestamp = Date::now();
    event.uids = bidRequest.userIds;

    sendEvent(event);
}


void
AppNexusWinSource::
sendClick(const BidRequest& bidRequest, const Bid& bid)
{
    PostAuctionEvent event;
    event.type = PAE_CAMPAIGN_EVENT;
    event.label = "CLICK";
    event.auctionId = bidRequest.auctionId;
    event.adSpotId = bid.adSpotId;
    event.timestamp = Date::now();
    event.uids = bidRequest.userIds;

    sendEvent(event);
}


void
AppNexusWinSource::
sendEvent(const PostAuctionEvent& event)
{
    std::string str = event.toJson().toString();
    std::string httpRequest = ML::format(
            "POST /win HTTP/1.1\r\n"
            "Content-Length: %zd\r\n"
            "Content-Type: application/json\r\n"
            "Connection: Keep-Alive\r\n"
            "\r\n"
            "%s",
            str.size(),
            str.c_str());

    write(httpRequest);

    std::string result = read();
    std::string status = "HTTP/1.1 200 OK";

    if(result.compare(0, status.length(), status)) {
        std::cerr << result << std::endl;
    }
}

namespace {

struct AtInit {
    AtInit()
    {
        WinSource::registerWinSourceFactory("appnexus", [](Json::Value const & json) {
            return new AppNexusWinSource(json);
        });
    }
} atInit;

}

