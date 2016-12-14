#include "../interface/PartonsFiller.h"

PartonsFiller::PartonsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(lheEventToken_, _cfg, _coll, "common", "lheEvent");
}

void
PartonsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  if (isRealData_)
    _eventBranches.push_back("!partons");
}

void
PartonsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  auto* lheEvent(getProductSafe_(_inEvent, lheEventToken_));
  if (!lheEvent)
    return;

  auto& outPartons(_outEvent.partons);

  auto& hepeup(lheEvent->hepeup());

  for (int iP(0); iP != hepeup.NUP; ++iP) {
    // only save status 1 partons
    if (hepeup.ISTUP[iP] != 1)
      continue;

    auto& outParton(outPartons.create_back());

    auto& pup(hepeup.PUP[iP]);
    outParton.setXYZE(pup[0], pup[1], pup[2], pup[3]);

    outParton.pdgid = hepeup.IDUP[iP];
  }
}

DEFINE_TREEFILLER(PartonsFiller);
