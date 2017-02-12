#include "../interface/MuonsFiller.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/Common/interface/RefToPtr.h"

MuonsFiller::MuonsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.)),
  maxEta_(getParameter_<double>(_cfg, "maxEta", 10.))
{
  getToken_(muonsToken_, _cfg, _coll, "muons");
  getToken_(verticesToken_, _cfg, _coll, "vertices", "vertices");

  if (useTrigger_) {
    for (unsigned iT(0); iT != panda::nMuonTriggerObjects; ++iT) {
      std::string name(panda::MuonTriggerObjectName[iT]); // "f<trigger filter name>"
      auto filters(getParameter_<VString>(_cfg, "triggerObjects." + name.substr(1)));
      triggerObjects_[iT].insert(filters.begin(), filters.end());
    }
  }
}

void
MuonsFiller::addOutput(TFile& _outputFile)
{
  TDirectory::TContext context(&_outputFile);
  auto* t(panda::makeMuonTriggerObjectTree());
  t->Write();
  delete t;
}

void
MuonsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("muons");

  if (isRealData_)
    _eventBranches.emplace_back("!muons.matchedGen_");
  if (!useTrigger_)
    _eventBranches.emplace_back("!muons.triggerMatch");
}

void
MuonsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inMuons(getProduct_(_inEvent, muonsToken_));
  auto& vertices(getProduct_(_inEvent, verticesToken_));

  auto& outMuons(_outEvent.muons);

  std::vector<edm::Ptr<reco::Muon>> ptrList;

  unsigned iMu(-1);
  for (auto& inMuon : inMuons) {
    ++iMu;

    if (inMuon.pt() < minPt_)
      continue;
    if (std::abs(inMuon.eta()) > maxEta_)
      continue;

    auto& outMuon(outMuons.create_back());

    fillP4(outMuon, inMuon);

    outMuon.q = inMuon.charge();

    auto& pfIso(inMuon.pfIsolationR04());

    outMuon.chiso = pfIso.sumChargedHadronPt;
    outMuon.nhiso = pfIso.sumNeutralHadronEt;
    outMuon.phoiso = pfIso.sumPhotonEt;
    outMuon.puiso = pfIso.sumPUPt;

    if (dynamic_cast<pat::Muon const*>(&inMuon)) {
      auto& patMuon(static_cast<pat::Muon const&>(inMuon));

      outMuon.loose = patMuon.isLooseMuon();
      outMuon.medium = patMuon.isMediumMuon();
      // Following the "short-term instruction for Moriond 2017" given in https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideMuonIdRun2#MediumID2016_to_be_used_with_Run
      // Valid only for runs B-F
      outMuon.mediumBtoF = outMuon.loose && inMuon.innerTrack()->validFraction() > 0.49 &&
        ((inMuon.isGlobalMuon() &&
          inMuon.globalTrack()->normalizedChi2() < 3. &&
          inMuon.combinedQuality().chi2LocalPosition < 12. &&
          inMuon.combinedQuality().trkKink < 20. &&
          muon::segmentCompatibility(inMuon) > 0.303) ||
         muon::segmentCompatibility(inMuon) > 0.451);
          
      if (vertices.size() == 0)
        outMuon.tight = false;
      else
        outMuon.tight = patMuon.isTightMuon(vertices.at(0));
    }
    else {
      outMuon.loose = muon::isLooseMuon(inMuon);
      outMuon.medium = muon::isMediumMuon(inMuon);
      if (vertices.size() == 0)
        outMuon.tight = false;
      else
        outMuon.tight = muon::isTightMuon(inMuon, vertices.at(0));
    }

    ptrList.push_back(inMuons.ptrAt(iMu));
  }
  
  auto originalIndices(outMuons.sort(panda::Particle::PtGreater));

  // export panda <-> reco mapping

  auto& muMuMap(objectMap_->get<reco::Muon, panda::Muon>());
  auto& genMuMap(objectMap_->get<reco::GenParticle, panda::Muon>());

  for (unsigned iP(0); iP != outMuons.size(); ++iP) {
    auto& outMuon(outMuons[iP]);
    unsigned idx(originalIndices[iP]);
    muMuMap.add(ptrList[idx], outMuon);

    if (!isRealData_) {
      auto& inMuon(*ptrList[idx]);

      if (dynamic_cast<pat::Muon const*>(&inMuon)) {
        auto& patMuon(static_cast<pat::Muon const&>(inMuon));
        auto ref(patMuon.genParticleRef());
        if (ref.isNonnull())
          genMuMap.add(edm::refToPtr(ref), outMuon);
      }
    }
  }
}

void
MuonsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  if (!isRealData_) {
    auto& genMuMap(objectMap_->get<reco::GenParticle, panda::Muon>());

    auto& genMap(_objectMaps.at("genParticles").get<reco::GenParticle, panda::GenParticle>().fwdMap);

    for (auto& link : genMuMap.bwdMap) {
      auto& genPtr(link.second);
      if (genMap.find(genPtr) == genMap.end())
        continue;

      auto& outMuon(*link.first);
      outMuon.matchedGen = genMap.at(genPtr);
    }
  }

  if (useTrigger_) {
    auto& objMap(_objectMaps.at("global").get<pat::TriggerObjectStandAlone, VString>().fwdMap);

    std::vector<pat::TriggerObjectStandAlone const*> triggerObjects[panda::nMuonTriggerObjects];

    // loop over the trigger filters we are interested in
    for (unsigned iT(0); iT != panda::nMuonTriggerObjects; ++iT) {
      // loop over all trigger objects (and their associated filter names)
      for (auto& objAndNames : objMap) { // (TO ptr, VString)
        VString const& names(*objAndNames.second);
        // loop over the associated filter names
        for (auto& name : names) {
          if (triggerObjects_[iT].find(name) != triggerObjects_[iT].end()) {
            triggerObjects[iT].push_back(&*objAndNames.first);
            break;
          }
        }
      }
    }

    auto& muMuMap(objectMap_->get<reco::Muon, panda::Muon>().fwdMap);

    for (auto& link : muMuMap) { // edm -> panda
      auto& inMuon(*link.first);
      auto& outMuon(*link.second);

      for (unsigned iT(0); iT != panda::nMuonTriggerObjects; ++iT) {
        for (auto* obj : triggerObjects[iT]) {
          if (reco::deltaR(inMuon, *obj) < 0.3) {
            outMuon.triggerMatch[iT] = true;
            break;
          }
        }
      }
    }
  }
}

DEFINE_TREEFILLER(MuonsFiller);
