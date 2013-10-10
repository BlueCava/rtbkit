/* appnexus_exchange_connector.cc
   Eric Robert, 23 July 2013
   
   Implementation of the AppNexus exchange connector.
*/

#include "appnexus_exchange_connector.h"
#include "rtbkit/plugins/bid_request/appnexus_bid_request.h"
#include "rtbkit/plugins/exchange/http_auction_handler.h"
/*
#include "rtbkit/common/testing/exchange_source.h"
#include "rtbkit/core/agent_configuration/agent_config.h"
#include "openrtb/openrtb_parsing.h"
#include "soa/types/json_printing.h"
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include "jml/utils/file_functions.h"
#include "jml/arch/info.h"
#include "jml/utils/rng.h"
*/

using namespace Datacratic;
/*
namespace Datacratic {

template<typename T, int I, typename S>
Json::Value jsonEncode(const ML::compact_vector<T, I, S> & vec)
{
    Json::Value result(Json::arrayValue);
    for (unsigned i = 0;  i < vec.size();  ++i)
        result[i] = jsonEncode(vec[i]);
    return result;
}

template<typename T, int I, typename S>
ML::compact_vector<T, I, S>
jsonDecode(const Json::Value & val, ML::compact_vector<T, I, S> *)
{
    ExcAssert(val.isArray());
    ML::compact_vector<T, I, S> res;
    res.reserve(val.size());
    for (unsigned i = 0;  i < val.size();  ++i)
        res.push_back(jsonDecode(val[i], (T*)0));
    return res;
}

} // namespace Datacratic
*/

namespace RTBKIT {

//BOOST_STATIC_ASSERT(hasFromJson<Datacratic::Id>::value == true);
//BOOST_STATIC_ASSERT(hasFromJson<int>::value == false);

/*****************************************************************************/
/* OPENRTB EXCHANGE CONNECTOR                                                */
/*****************************************************************************/

AppNexusExchangeConnector::
AppNexusExchangeConnector(ServiceBase & owner, const std::string & name)
    : HttpExchangeConnector(name, owner)
{
}

AppNexusExchangeConnector::
AppNexusExchangeConnector(const std::string & name,
                          std::shared_ptr<ServiceProxies> proxies)
    : HttpExchangeConnector(name, proxies)
{
}

std::shared_ptr<BidRequest>
AppNexusExchangeConnector::
parseBidRequest(HttpAuctionHandler & connection,
                const HttpHeader & header,
                const std::string & payload)
{
    std::shared_ptr<BidRequest> result;

#if 0
	std::cout << "AppNexusExchangeConnector::parseBidRequest " << payload.c_str() << std::endl;
#endif    

    auto name = exchangeNameString(); 

    // Parse the bid request
    ML::Parse_Context context("Bid Request", payload.c_str(), payload.size());
    result.reset(AppNexusBidRequestParser::parseBidRequest(context, name, name));
    return result;
}

double
AppNexusExchangeConnector::
getTimeAvailableMs(HttpAuctionHandler & connection,
                   const HttpHeader & header,
                   const std::string & payload)
{
    // Scan the payload quickly for the tmax parameter.
    static const std::string toFind = "\"bidder_timeout_ms\":";
    std::string::size_type pos = payload.find(toFind);
    if (pos == std::string::npos)
        return 100.0;
    
    int tmax = atoi(payload.c_str() + pos + toFind.length());
    return tmax;
}

HttpResponse
AppNexusExchangeConnector::
getResponse(const HttpAuctionHandler & connection,
            const HttpHeader & requestHeader,
            const Auction & auction) const
{
    const Auction::Data * current = auction.getCurrentData();

    if (current->hasError())
        return getErrorResponse(connection, auction,
                                current->error + ": " + current->details);

	AppNexus::BidResponseRoot appnexusBidResponse;
	for(auto idx = 0; idx < current->responses.size(); ++idx)	{

		appnexusBidResponse.responses.emplace_back();
		auto &bidResponse = appnexusBidResponse.responses.back();
		bidResponse.memberId.val = auction.request->ext["memberId"].asInt();
		bidResponse.auctionId.val = auction.id.toInt();
		bidResponse.exclusive.val = false;
		bidResponse.noBid.val = true;

		if (current->hasValidResponse(idx))	{		
			auto & resp = current->winningResponse(idx);
			bidResponse.noBid.val = false;
			bidResponse.price.val = USD_CPM(resp.price.maxPrice);			

			auto respMeta = Json::parse(resp.meta);
			bidResponse.creativeId.val = respMeta["creativeId"].asInt();
			bidResponse.pixelUrl = respMeta["pixelUrl"].asString();
			bidResponse.pixelType = respMeta["pixelType"].asString();

			Json::Value customData;
			customData["accountId"] = resp.account.toString();
			customData["bidSpotIndex"] = respMeta["bidSpotIndex"];
			bidResponse.customNotifyData = customData.toString();

			//TODO: might be useful to set data on AppNexus cookies.
			//bidResponse.userDataJS = "";			
		}
	}

    static Datacratic::DefaultDescription<AppNexus::BidResponseRoot> desc;
    std::ostringstream stream;
    StreamJsonPrintingContext context(stream);
    desc.printJsonTyped(&appnexusBidResponse, context);

	string strResponse = "{\"bid_response\": " + stream.str() + "}";

#if 0
	std::cout << strResponse << std::endl;
#endif

	return HttpResponse(200, "application/json", strResponse);
}

HttpResponse
AppNexusExchangeConnector::
getDroppedAuctionResponse(const HttpAuctionHandler & connection,
                          const std::string & reason) const
{
	return HttpResponse(204, "application/json", "{\"bid_response\": {\"no_bid\": true}}");
}

HttpResponse
AppNexusExchangeConnector::
getErrorResponse(const HttpAuctionHandler & connection,
                 const Auction & auction,
                 const std::string & errorMessage) const
{
    Json::Value response;
    response["error"] = errorMessage;
    return HttpResponse(400, response);
}

void
AppNexusExchangeConnector::
handleUnknownRequest(HttpAuctionHandler & connection,
                     const HttpHeader & header,
                     const std::string & payload) const
{
#if 0
	if (header.resource == "/ready") {
        connection.putResponseOnWire(HttpResponse(200, "text/plain", "1"));
        return;
    }

	std::cout << "Appnexus::handleUnknownRequest" << std::endl;

	std::cout << "[verb]: [" << header.verb << "]" << std::endl;
	std::cout << "[resource]: [" << header.resource << "]" << std::endl;
	std::cout << "[version]: [" << header.version << "]" << std::endl;
	std::cout << "[contentType]: [" << header.contentType << "]" << std::endl;
	std::cout << "[contentLength]: [" << header.contentLength << "]" << std::endl;
	std::cout << "[knownData]: [" << header.knownData << "]" << std::endl;
	std::cout << "[queryParams]: [" << header.queryParams.uriEscaped() << "]" << std::endl;

	for (auto iter = header.headers.begin(); iter != header.headers.end(); ++iter) {
		std::cout << "[" << iter->first << "]: [" << iter->second << "]" << std::endl;
	}

	connection.sendErrorResponse("unknown resource " + header.resource);

	return;
#endif

	HttpExchangeConnector::handleUnknownRequest(connection, header, payload);
}

} // namespace RTBKIT

namespace {
    using namespace RTBKIT;

    struct Init {
        Init() {
            ExchangeConnector::registerFactory<AppNexusExchangeConnector>();
        }
    } init;
}

