#ifndef PandaProd_Producer_PFCandsFiller_h
#define PandaProd_Producer_PFCandsFiller_h

#include "FillerBase.h"

#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/Common/interface/ValueMap.h"

class PFCandsFiller : public FillerBase {
 public:
  PFCandsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~PFCandsFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  typedef edm::ValueMap<float> FloatMap;

  NamedToken<reco::CandidateView> candidatesToken_;
  NamedToken<FloatMap> puppiMapToken_;
  NamedToken<FloatMap> puppiNoLepMapToken_;
};

#endif
