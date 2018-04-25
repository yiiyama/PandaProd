#include "../interface/MetFiller.h"

#include "DataFormats/PatCandidates/interface/MET.h"

MetFiller::MetFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  if (_name == "pfMet")
    outputSelector_ = [](panda::Event& _event)->panda::RecoMet& { return _event.pfMet; };
  else if (_name == "puppiMet")
    outputSelector_ = [](panda::Event& _event)->panda::RecoMet& { return _event.puppiMet; };
  else
    throw edm::Exception(edm::errors::Configuration, "Unknown MET output");

  getToken_(metToken_, _cfg, _coll, "met");
}

void
MetFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back(getName());

  if (isRealData_) {
    char const* skipped[] = {
      ".ptSmear",
      ".ptSmearUp",
      ".ptSmearDown",
      ".phiSmear",
      ".phiSmearUp",
      ".phiSmearDown"
    };

    for (char const* b : skipped)
      _eventBranches.emplace_back("!" + getName() + b);
  }
}

void
MetFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inMet(getProduct_(_inEvent, metToken_).at(0));

  panda::RecoMet& outMet(outputSelector_(_outEvent));

  outMet.pt = inMet.pt();
  outMet.phi = inMet.phi();

  auto* patMet(dynamic_cast<pat::MET const*>(&inMet));

  if (patMet) {
    outMet.sumETRaw = patMet->uncorSumEt();

    outMet.ptCorrUp = patMet->shiftedPt(pat::MET::JetEnUp);
    outMet.phiCorrUp = patMet->shiftedPhi(pat::MET::JetEnUp);
    outMet.ptCorrDown = patMet->shiftedPt(pat::MET::JetEnDown);
    outMet.phiCorrDown = patMet->shiftedPhi(pat::MET::JetEnDown);

    outMet.ptUnclUp = patMet->shiftedPt(pat::MET::UnclusteredEnUp);
    outMet.phiUnclUp = patMet->shiftedPhi(pat::MET::UnclusteredEnUp);
    outMet.ptUnclDown = patMet->shiftedPt(pat::MET::UnclusteredEnDown);
    outMet.phiUnclDown = patMet->shiftedPhi(pat::MET::UnclusteredEnDown);
    outMet.significance = patMet->metSignificance();
  }
}

DEFINE_TREEFILLER(MetFiller);
