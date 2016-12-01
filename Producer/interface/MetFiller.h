#ifndef PandaProd_Producer_MetFiller_h
#define PandaProd_Producer_MetFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/METReco/interface/MET.h"

class MetFiller : public FillerBase {
 public:
  MetFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~MetFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  typedef edm::View<reco::MET> METView;

  edm::EDGetTokenT<METView> metToken_;
  edm::EDGetTokenT<reco::CandidateView> candidatesToken_;
};

#endif
