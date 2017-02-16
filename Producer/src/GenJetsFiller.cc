#include "../interface/GenJetsFiller.h"

GenJetsFiller::GenJetsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.))
{
  getToken_(genJetsToken_, _cfg, _coll, "genJets");
  getToken_(flavorToken_, _cfg, _coll, "flavor");

  if (_name == "ak4GenJets")
    outputSelector_ = [](panda::Event& _event)->panda::GenJetCollection& { return _event.ak4GenJets; };
  else if (_name == "ak8GenJets")
    outputSelector_ = [](panda::Event& _event)->panda::GenJetCollection& { return _event.ak8GenJets; };
  else if (_name == "ca15GenJets")
    outputSelector_ = [](panda::Event& _event)->panda::GenJetCollection& { return _event.ca15GenJets; };
  else
    throw edm::Exception(edm::errors::Configuration, "Unknown GenJetCollection output");    
}

void
GenJetsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back(getName());
}

void
GenJetsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  auto& inJets(getProduct_(_inEvent, genJetsToken_));
  auto& inFlavor(getProduct_(_inEvent, flavorToken_));

  auto& outJets(outputSelector_(_outEvent));

  std::vector<edm::Ptr<reco::GenJet>> ptrList;

  unsigned iJet(-1);
  for (auto& inJet : inJets) {
    ++iJet;
    if (inJet.pt() < minPt_)
      continue;

    auto& outJet(outJets.create_back());

    fillP4(outJet, inJet);
    
    auto&& ptr(inJets.ptrAt(iJet));

    outJet.pdgid = inFlavor[edm::Ptr<reco::Jet>(ptr)].getHadronFlavour();

    ptrList.push_back(ptr);
  }

  // sort the output electrons
  auto originalIndices(outJets.sort(panda::Particle::PtGreater));

  // make reco <-> panda mapping
  auto& objectMap(objectMap_->get<reco::GenJet, panda::GenJet>());
  
  for (unsigned iP(0); iP != outJets.size(); ++iP) {
    auto& outJet(outJets[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outJet);
  }
}

DEFINE_TREEFILLER(GenJetsFiller);
