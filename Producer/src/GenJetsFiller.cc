#include "../interface/GenJetsFiller.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

GenJetsFiller::GenJetsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.))
{
  getToken_(genJetsToken_, _cfg, _coll, "genJets");
  getToken_(flavorToken_, _cfg, _coll, "flavor");
  getToken_(genParticlesToken_, _cfg, _coll, "common", "genParticles");
  
  getToken_(genBHadPlusMothersToken_       , _cfg, _coll, "genBHadPlusMothers"        , false);
  //getToken_(genBHadPlusMothersIndicesToken_, _cfg, _coll, "genBHadPlusMothersIndices" , false);
  getToken_(genBHadIndexToken_             , _cfg, _coll, "genBHadIndex"              , false);
  //getToken_(genBHadFlavourToken_           , _cfg, _coll, "genBHadFlavour"            , false);
  getToken_(genBHadJetIndexToken_          , _cfg, _coll, "genBHadJetIndex"           , false);
  //getToken_(genBHadLeptonIndexToken_       , _cfg, _coll, "genBHadLeptonIndex"        , false);
  //getToken_(genBHadLeptonHadronIndexToken_ , _cfg, _coll, "genBHadLeptonHadronIndex"  , false);
  //getToken_(genBHadLeptonViaTauToken_      , _cfg, _coll, "genBHadLeptonViaTau"       , false);
  //getToken_(genBHadFromTopWeakDecayToken_  , _cfg, _coll, "genBHadFromTopWeakDecay"   , false);
  //getToken_(genBHadBHadronIdToken_         , _cfg, _coll, "genBHadBHadronId"          , false);
  
  getToken_(genCHadPlusMothersToken_       , _cfg, _coll, "genCHadPlusMothers"        , false);
  //getToken_(genCHadPlusMothersIndicesToken_, _cfg, _coll, "genCHadPlusMothersIndices" , false);
  getToken_(genCHadIndexToken_             , _cfg, _coll, "genCHadIndex"              , false);
  //getToken_(genCHadFlavourToken_           , _cfg, _coll, "genCHadFlavour"            , false);
  getToken_(genCHadJetIndexToken_          , _cfg, _coll, "genCHadJetIndex"           , false);
  //getToken_(genCHadLeptonIndexToken_       , _cfg, _coll, "genCHadLeptonIndex"        , false);
  //getToken_(genCHadLeptonHadronIndexToken_ , _cfg, _coll, "genCHadLeptonHadronIndex"  , false);
  //getToken_(genCHadLeptonViaTauToken_      , _cfg, _coll, "genCHadLeptonViaTau"       , false);
  //getToken_(genCHadFromTopWeakDecayToken_  , _cfg, _coll, "genCHadFromTopWeakDecay"   , false);
  //getToken_(genCHadBHadronIdToken_         , _cfg, _coll, "genCHadBHadronId"          , false);

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
  auto& inParticles(getProduct_(_inEvent, genParticlesToken_));
  
  auto *genBHadPlusMothers       (getProductSafe_(_inEvent, genBHadPlusMothersToken_       ));
  //auto *genBHadPlusMothersIndices(getProductSafe_(_inEvent, genBHadPlusMothersIndicesToken_));
  auto *genBHadIndex             (getProductSafe_(_inEvent, genBHadIndexToken_             ));
  //auto *genBHadFlavour           (getProductSafe_(_inEvent, genBHadFlavourToken_           ));
  auto *genBHadJetIndex          (getProductSafe_(_inEvent, genBHadJetIndexToken_          ));
  //auto *genBHadLeptonIndex       (getProductSafe_(_inEvent, genBHadLeptonIndexToken_       ));
  //auto *genBHadLeptonHadronIndex (getProductSafe_(_inEvent, genBHadLeptonHadronIndexToken_ ));
  //auto *genBHadLeptonViaTau      (getProductSafe_(_inEvent, genBHadLeptonViaTauToken_      ));
  //auto *genBHadFromTopWeakDecay  (getProductSafe_(_inEvent, genBHadFromTopWeakDecayToken_  ));
  //auto *genBHadBHadronId         (getProductSafe_(_inEvent, genBHadBHadronIdToken_         ));
  
  auto *genCHadPlusMothers       (getProductSafe_(_inEvent, genCHadPlusMothersToken_       ));
  //auto *genCHadPlusMothersIndices(getProductSafe_(_inEvent, genCHadPlusMothersIndicesToken_));
  auto *genCHadIndex             (getProductSafe_(_inEvent, genCHadIndexToken_             ));
  //auto *genCHadFlavour           (getProductSafe_(_inEvent, genCHadFlavourToken_           ));
  auto *genCHadJetIndex          (getProductSafe_(_inEvent, genCHadJetIndexToken_          ));
  //auto *genCHadLeptonIndex       (getProductSafe_(_inEvent, genCHadLeptonIndexToken_       ));
  //auto *genCHadLeptonHadronIndex (getProductSafe_(_inEvent, genCHadLeptonHadronIndexToken_ ));
  //auto *genCHadLeptonViaTau      (getProductSafe_(_inEvent, genCHadLeptonViaTauToken_      ));
  //auto *genCHadFromTopWeakDecay  (getProductSafe_(_inEvent, genCHadFromTopWeakDecayToken_  ));
  //auto *genCHadBHadronId         (getProductSafe_(_inEvent, genCHadBHadronIdToken_         ));

  auto& outJets(outputSelector_(_outEvent));

  std::vector<edm::Ptr<reco::GenJet>> ptrList;

  unsigned iJet(-1);
  jetBHadrons.clear();
  jetCHadrons.clear();
  for (auto& inJet : inJets) {
    ++iJet;
    if (inJet.pt() < minPt_)
      continue;

    auto& outJet(outJets.create_back());

    fillP4(outJet, inJet);
    
    auto&& ptr(inJets.ptrAt(iJet));

    auto& flavor(inFlavor[edm::Ptr<reco::Jet>(ptr)]);
    
    if (genBHadJetIndex) for (unsigned genBHad=0; genBHad<genBHadJetIndex->size(); genBHad++) {
      unsigned jetIndex = genBHadJetIndex->at(genBHad);
      unsigned hadIndex = genBHadIndex->at(genBHad);
      const reco::GenParticle* genHadron=0; // This will hold a pointer to the (duplicated) gen particle hadron mother
      if (jetIndex == iJet) genHadron = &(genBHadPlusMothers->at(hadIndex));
      // Find the corresponding pointer in the true gen particle collection
      if(genHadron) for (unsigned iP(0); iP != inParticles.size(); ++iP) {
        auto& inCand(inParticles.at(iP));
        edm::Ptr<reco::Candidate> inParticlesPtr(inParticles.ptrAt(iP));
        if (
          inCand.pdgId()              == genHadron->pdgId() &&
          inCand.statusFlags().flags_ == genHadron->statusFlags().flags_ &&
          inCand.p4()                 == genHadron->p4()
        ) {
          jetBHadrons[ptr].push_back(inParticlesPtr);
          break;
        }
      }
    }
    if (genCHadJetIndex) for (unsigned genCHad=0; genCHad<genCHadJetIndex->size(); genCHad++) {
      unsigned jetIndex = genCHadJetIndex->at(genCHad);
      unsigned hadIndex = genCHadIndex->at(genCHad);
      const reco::GenParticle* genHadron=0; // This will hold a pointer to the (duplicated) gen particle hadron mother
      if (jetIndex == iJet) genHadron = &(genCHadPlusMothers->at(hadIndex));
      // Find the corresponding pointer in the true gen particle collection
      if(genHadron) for (unsigned iP(0); iP != inParticles.size(); ++iP) {
        auto& inCand(inParticles.at(iP));
        edm::Ptr<reco::Candidate> inParticlesPtr(inParticles.ptrAt(iP));
        if (
          inCand.pdgId()              == genHadron->pdgId() &&
          inCand.statusFlags().flags_ == genHadron->statusFlags().flags_ &&
          inCand.p4()                 == genHadron->p4()
        ) {
          jetCHadrons[ptr].push_back(inParticlesPtr);
          break;
        }
      }
    }
    
    outJet.pdgid = flavor.getHadronFlavour();
    outJet.partonFlavor = flavor.getPartonFlavour();
    //outJet.numB = flavor.getbHadrons().size();
    //outJet.numC = flavor.getcHadrons().size();
    outJet.numB = jetBHadrons[ptr].size();
    outJet.numC = jetCHadrons[ptr].size();

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

void
GenJetsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  auto& genJetMap(objectMap_->get<reco::GenJet, panda::GenJet>().fwdMap);
  auto& genParticleMap(_objectMaps.at("genParticles").get<reco::Candidate, panda::GenParticle>().fwdMap);
  // For each gen jet
  if (jetBHadrons.size()>0) for (auto const& jetBHadronMapping : jetBHadrons) {
    // Dig up the corresponding reference to the panda::GenJet
    auto matchedJetPtr = jetBHadronMapping.first;
    auto&& genJetLink(genJetMap.find(matchedJetPtr));
    //auto &inGenJet(*genJetLink->first);
    auto &outGenJet(*genJetLink->second);    
    const std::vector<edm::Ptr<reco::Candidate> > *matchedBHadrons = &jetBHadronMapping.second;
    for (unsigned iHad=0; iHad<matchedBHadrons->size(); iHad++) {
      auto&& outGenParticleLink(genParticleMap.find(matchedBHadrons->at(iHad)));
      if(outGenParticleLink != genParticleMap.end())
        outGenJet.matchedBHadrons.addRef(outGenParticleLink->second);
      else
        edm::LogWarning("GenJetsFiller") << "Could not add reference to gen particle (iHad="<<iHad<<") looking in map with size "<<genParticleMap.size()<<"\n";
    }
  }
  if (jetCHadrons.size()>0) for (auto const& jetCHadronMapping : jetCHadrons) {
    // Dig up the corresponding reference to the panda::GenJet
    auto matchedJetPtr = jetCHadronMapping.first;
    auto&& genJetLink(genJetMap.find(matchedJetPtr));
    //auto &inGenJet(*genJetLink->first);
    auto &outGenJet(*genJetLink->second);
    const std::vector<edm::Ptr<reco::Candidate> > *matchedCHadrons = &jetCHadronMapping.second;
    for (unsigned iHad=0; iHad<matchedCHadrons->size(); iHad++) {
      auto&& outGenParticleLink(genParticleMap.find(matchedCHadrons->at(iHad)));
      if(outGenParticleLink != genParticleMap.end())
        outGenJet.matchedCHadrons.addRef(outGenParticleLink->second);
      else
        edm::LogWarning("GenJetsFiller") << "Could not add reference to gen particle (iHad="<<iHad<<") looking in map with size "<<genParticleMap.size()<<"\n";
    }
        
  }
}

DEFINE_TREEFILLER(GenJetsFiller);
