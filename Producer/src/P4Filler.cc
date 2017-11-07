#include "../interface/P4Filler.h"

#include "DataFormats/PatCandidates/interface/Jet.h"

P4Filler::P4Filler(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(sourceToken_, _cfg, _coll, "source");

  if (_name == "softTrackJets")
    outputSelector_ = [](panda::Event& _event)->panda::ParticleCollection& { return _event.softTrackJets; };

}

void
P4Filler::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back(getName());
}

void
P4Filler::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{

  auto& inVecs(getProduct_(_inEvent, sourceToken_));
  auto& outVecs(outputSelector_(_outEvent));

  for (auto& inVec : inVecs) {

    auto& outVec(outVecs.create_back());
    fillP4(outVec, inVec);

  }
}

DEFINE_TREEFILLER(P4Filler);
