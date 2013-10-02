/** appnexus_mock_exchange.h                                 -*- C++ -*-
    Appnexus Mock exchanges used for testing various types of bid request behaviours.

    \todo This a work in progress that needs to be generic-ified.

*/

#ifndef __rtbkit__appnexus_mock_exchange_h__
#define __rtbkit__appnexus_mock_exchange_h__

#include "rtbkit/common/testing/exchange_source.h"
#include "soa/service/service_utils.h"
#include "soa/service/service_base.h"
#include "soa/service/zmq_endpoint.h"
#include "soa/types/id.h"
#include "soa/types/date.h"
#include "jml/utils/rng.h"

#include <netdb.h>
#include <vector>

namespace RTBKIT {

/******************************************************************************/
/* Appnexus MOCK EXCHANGE                                                              */
/******************************************************************************/

struct AppnexusMockExchange : public Datacratic::ServiceBase
{
    AppnexusMockExchange(Datacratic::ServiceProxyArguments & args,
                 const std::string& name = "appnexus-mock-exchange");
    AppnexusMockExchange(const std::shared_ptr<Datacratic::ServiceProxies> services,
                 const std::string& name = "appnexus-mock-exchange");

    ~AppnexusMockExchange();

    void start(Json::Value const & configuration);

    bool isDone() const {
        return !running;
    }

    void add(BidSource * bids, WinSource * wins);

private:
    int running;

    struct Worker {
        Worker(AppnexusMockExchange * exchange, BidSource * bid, WinSource * win);
        Worker(AppnexusMockExchange * exchange, Json::Value bid, Json::Value win);

        void run();
        bool bid();

        std::pair<bool, Amount>
        isWin(const BidRequest&, const ExchangeSource::Bid& bid);
        bool isClick(const BidRequest&, const ExchangeSource::Bid& bid);

        AppnexusMockExchange * exchange;
        std::unique_ptr<BidSource> bids;
        std::unique_ptr<WinSource> wins;
        ML::RNG rng;
    };

    std::vector<Worker> workers;
    boost::thread_group threads;
};


} // namespace RTBKIT

#endif // __rtbkit__appnexus_mock_exchange_h__
