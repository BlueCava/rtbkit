/*
  Code to parse AppNexus bid requests.
*/

#pragma once

#include <string>
#include "appnexus.h"
#include "openrtb/openrtb_parsing.h"

// using namespace OpenRTB;
using std::string;


namespace Datacratic {


  template<>
  struct DefaultDescription<AppNexus::AdPosition>
    : public TaggedEnumDescription<AppNexus::AdPosition> {

    DefaultDescription()
    {
    }

    void parseJsonTyped(AppNexus::AdPosition * val,
			JsonParsingContext & context) const
    {
      string appNexAdPos = context.expectStringAscii();

      if (appNexAdPos == "unknown") {
	val->val = AppNexus::AdPosition::UNKNOWN;
      } 
      else if (appNexAdPos == "above") {
	val->val = AppNexus::AdPosition::ABOVE;
      } 
      else if (appNexAdPos == "below") {
	val->val = AppNexus::AdPosition::BELOW;
      }
      else { // AN only supports the above three AdPos types.
	// ORTB supports others but AN does not.
	val->val = AppNexus::AdPosition::UNSPECIFIED;
      }
    }
  };


  template<>
  struct DefaultDescription<AppNexus::BidRequestRoot>
    : public StructureDescription<AppNexus::BidRequestRoot> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::BidRequest>
    : public StructureDescription<AppNexus::BidRequest> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::BidInfo>
    : public StructureDescription<AppNexus::BidInfo> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::DeviceIds>
    : public StructureDescription<AppNexus::DeviceIds> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::Segment>
    : public StructureDescription<AppNexus::Segment> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::InventoryAudit>
    : public StructureDescription<AppNexus::InventoryAudit> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::Tag>
    : public StructureDescription<AppNexus::Tag> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::Member>
    : public StructureDescription<AppNexus::Member> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::BidResponse>
    : public StructureDescription<AppNexus::BidResponse> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::BidResponseRoot>
    : public StructureDescription<AppNexus::BidResponseRoot> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::NotifyTag>
    : public StructureDescription<AppNexus::NotifyTag> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::NotifyResponse>
    : public StructureDescription<AppNexus::NotifyResponse> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::NotifyRequest>
    : public StructureDescription<AppNexus::NotifyRequest> {
    DefaultDescription();
  };

  template<>
  struct DefaultDescription<AppNexus::NotifyRequestRoot>
    : public StructureDescription<AppNexus::NotifyRequestRoot> {
    DefaultDescription();
  };


} // namespace Datacratic
