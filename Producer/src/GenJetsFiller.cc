#include "../interface/GenJetsFiller.h"

GenJetsFiller::GenJetsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.))
{
  getToken_(genJetsToken_, _cfg, _coll, "genJets");
}

void
GenJetsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  if (!enabled())
    _eventBranches.emplace_back("!" + getName());
}

void
GenJetsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  auto& inJets(getProduct_(_inEvent, genJetsToken_));

  auto& outJets(_outEvent.genJets);

  std::vector<edm::Ptr<reco::GenJet>> ptrList;

  unsigned iJet(-1);
  for (auto& inJet : inJets) {
    ++iJet;
    if (inJet.pt() < minPt_)
      continue;

    auto& outJet(outJets.create_back());

    fillP4(outJet, inJet);

    outJet.pdgid = inJet.pdgId();

    ptrList.push_back(inJets.ptrAt(iJet));
  }

  // sort the output electrons
  auto originalIndices(outJets.sort(panda::ptGreater));

  // make reco <-> panda mapping
  auto& objectMap(objectMap_->get<reco::GenJet, panda::PGenJet>());
  
  for (unsigned iP(0); iP != outJets.size(); ++iP) {
    auto& outJet(outJets[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outJet);
  }
}

DEFINE_TREEFILLER(GenJetsFiller);
