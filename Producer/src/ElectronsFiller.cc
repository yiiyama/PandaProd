#include "../interface/ElectronsFiller.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/HepMCCandidate/interface/GenStatusFlags.h"
#include "DataFormats/Math/interface/deltaR.h"

#include <cmath>

ElectronsFiller::ElectronsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  combIsoEA_(getParameter_<edm::FileInPath>(_cfg, "combIsoEA").fullPath()),
  ecalIsoEA_(getParameter_<edm::FileInPath>(_cfg, "ecalIsoEA").fullPath()),
  hcalIsoEA_(getParameter_<edm::FileInPath>(_cfg, "hcalIsoEA").fullPath()),
  phCHIsoEA_(getFillerParameter_<edm::FileInPath>(_cfg, "photons", "chIsoEA").fullPath()),
  phNHIsoEA_(getFillerParameter_<edm::FileInPath>(_cfg, "photons", "nhIsoEA").fullPath()),
  phPhIsoEA_(getFillerParameter_<edm::FileInPath>(_cfg, "photons", "phIsoEA").fullPath()),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.)),
  maxEta_(getParameter_<double>(_cfg, "maxEta", 10.))
{
  getToken_(electronsToken_, _cfg, _coll, "electrons");
  getToken_(smearedElectronsToken_, _cfg, _coll, "smearedElectrons");
  getToken_(regressionElectronsToken_, _cfg, _coll, "regressionElectrons");
  getToken_(gsUnfixedElectronsToken_, _cfg, _coll, "gsUnfixedElectrons", false);
  getToken_(photonsToken_, _cfg, _coll, "photons", "photons");
  getToken_(pfCandidatesToken_, _cfg, _coll, "common", "pfCandidates");
  getToken_(ebHitsToken_, _cfg, _coll, "common", "ebHits");
  getToken_(eeHitsToken_, _cfg, _coll, "common", "eeHits");
  getToken_(vetoIdToken_, _cfg, _coll, "vetoId");
  getToken_(looseIdToken_, _cfg, _coll, "looseId");
  getToken_(mediumIdToken_, _cfg, _coll, "mediumId");
  getToken_(tightIdToken_, _cfg, _coll, "tightId");
  getToken_(hltIdToken_, _cfg, _coll, "hltId");
  getToken_(phCHIsoToken_, _cfg, _coll, "photons", "chIso");
  getToken_(phNHIsoToken_, _cfg, _coll, "photons", "nhIso");
  getToken_(phPhIsoToken_, _cfg, _coll, "photons", "phIso");
  getToken_(ecalIsoToken_, _cfg, _coll, "ecalIso", false);
  getToken_(hcalIsoToken_, _cfg, _coll, "hcalIso", false);
  getToken_(rhoToken_, _cfg, _coll, "rho", "rho");
  getToken_(rhoCentralCaloToken_, _cfg, _coll, "rho", "rhoCentralCalo");

  if (useTrigger_) {
    for (unsigned iT(0); iT != panda::Electron::nTriggerObjects; ++iT) {
      std::string name(panda::Electron::TriggerObjectName[iT]); // "f<trigger filter name>"
      auto filters(getParameter_<VString>(_cfg, "triggerObjects." + name.substr(1)));
      triggerObjects_[iT].insert(filters.begin(), filters.end());
    }
  }
}

void
ElectronsFiller::addOutput(TFile& _outputFile)
{
  TDirectory::TContext context(&_outputFile);
  auto* t(panda::utils::makeDocTree("ElectronTriggerObject", panda::Electron::TriggerObjectName, panda::Electron::nTriggerObjects));
  t->Write();
  delete t;
}

void
ElectronsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("electrons");

  if (isRealData_)
    _eventBranches.emplace_back("!electrons.matchedGen_");
  if (!useTrigger_)
    _eventBranches.emplace_back("!electrons.triggerMatch");
  if (gsUnfixedElectronsToken_.second.isUninitialized())
    _eventBranches.emplace_back("!electrons.originalPt");
}

void
ElectronsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inElectrons(getProduct_(_inEvent, electronsToken_));
  auto& inSmearedElectrons(getProduct_(_inEvent, smearedElectronsToken_));
  auto& inRegressionElectrons(getProduct_(_inEvent, regressionElectronsToken_));
  auto* gsUnfixedElectrons(getProductSafe_(_inEvent, gsUnfixedElectronsToken_));
  auto& photons(getProduct_(_inEvent, photonsToken_));
  auto& pfCandidates(getProduct_(_inEvent, pfCandidatesToken_));
  auto& ebHits(getProduct_(_inEvent, ebHitsToken_));
  auto& eeHits(getProduct_(_inEvent, eeHitsToken_));
  auto& vetoId(getProduct_(_inEvent, vetoIdToken_));
  auto& looseId(getProduct_(_inEvent, looseIdToken_));
  auto& mediumId(getProduct_(_inEvent, mediumIdToken_));
  auto& tightId(getProduct_(_inEvent, tightIdToken_));
  auto& hltId(getProduct_(_inEvent, hltIdToken_));
  auto& phCHIso(getProduct_(_inEvent, phCHIsoToken_));
  auto& phNHIso(getProduct_(_inEvent, phNHIsoToken_));
  auto& phPhIso(getProduct_(_inEvent, phPhIsoToken_));
  FloatMap const* ecalIso(0);
  if (!ecalIsoToken_.second.isUninitialized())
    ecalIso = &getProduct_(_inEvent, ecalIsoToken_);
  FloatMap const* hcalIso(0);
  if (!hcalIsoToken_.second.isUninitialized())
    hcalIso = &getProduct_(_inEvent, hcalIsoToken_);
  double rho(getProduct_(_inEvent, rhoToken_));
  double rhoCentralCalo(getProduct_(_inEvent, rhoCentralCaloToken_));

  auto findHit([&ebHits, &eeHits](DetId const& id)->EcalRecHit const* {
      EcalRecHitCollection const* hits(0);
      if (id.subdetId() == EcalBarrel)
        hits = &ebHits;
      else
        hits = &eeHits;

      auto&& hitItr(hits->find(id));
      if (hitItr == hits->end())
        return 0;

      return &*hitItr;
    });

  auto findPF([&pfCandidates](reco::GsfElectron const& inElectron)->reco::CandidatePtr {
      int iMatch(-1);

      double minDR(0.1);
      for (unsigned iPF(0); iPF != pfCandidates.size(); ++iPF) {
        auto& pf(pfCandidates.at(iPF));
        if (std::abs(pf.pdgId()) != 11 || pf.pdgId() == 22)
          continue;

        double dR(reco::deltaR(pf, inElectron));
        if (dR < minDR) {
          minDR = dR;
          iMatch = iPF;
        }
      }

      if (iMatch >= 0)
        return pfCandidates.ptrAt(iMatch);
      else
        return reco::CandidatePtr();
    });

  auto& outElectrons(_outEvent.electrons);

  std::vector<edm::Ptr<reco::GsfElectron>> ptrList;

  // See below
  // std::map<uint32_t, reco::GsfElectron const*> gsFixMap;
  // if (gsUnfixedElectrons) {
  //   for (auto& ele : *gsUnfixedElectrons) {
  //     auto&& scRef(ele.superCluster());
  //     if (scRef.isNull())
  //       continue;
  //     auto&& bcRef(scRef->seed());
  //     if (bcRef.isNull())
  //       continue;

  //     gsFixMap[bcRef->seed().rawId()] = &ele;
  //   }
  // }

  unsigned iEl(-1);
  for (auto& inElectron : inElectrons) {
    ++iEl;
    if (inElectron.pt() < minPt_)
      continue;
    if (std::abs(inElectron.eta()) > maxEta_)
      continue;

    auto&& inRef(inElectrons.refAt(iEl));

    bool veto(vetoId[inRef]);

    if (!veto)
      continue;
    
    auto&& scRef(inElectron.superCluster());
    auto& sc(*scRef);

    auto& outElectron(outElectrons.create_back());

    fillP4(outElectron, inElectron);

    outElectron.veto = veto;
    outElectron.loose = looseId[inRef];
    outElectron.medium = mediumId[inRef];
    outElectron.tight = tightId[inRef];
    outElectron.hltsafe = hltId[inRef];

    outElectron.charge = inElectron.charge();

    outElectron.sieie = inElectron.full5x5_sigmaIetaIeta();
    outElectron.sipip = inElectron.full5x5_sigmaIphiIphi();
    outElectron.hOverE = inElectron.hadronicOverEm();

    double scEta(std::abs(sc.eta()));

    auto& pfIso(inElectron.pfIsolationVariables());
    outElectron.chIso = pfIso.sumChargedHadronPt;
    outElectron.nhIso = pfIso.sumNeutralHadronEt;
    outElectron.phIso = pfIso.sumPhotonEt;
    outElectron.puIso = pfIso.sumPUPt;
    outElectron.isoPUOffset = combIsoEA_.getEffectiveArea(scEta) * rho;

    if (dynamic_cast<pat::Electron const*>(&inElectron)) {
      auto& patElectron(static_cast<pat::Electron const&>(inElectron));
      outElectron.ecalIso = patElectron.ecalPFClusterIso() - ecalIsoEA_.getEffectiveArea(scEta) * rhoCentralCalo;
      outElectron.hcalIso = patElectron.hcalPFClusterIso() - hcalIsoEA_.getEffectiveArea(scEta) * rhoCentralCalo;
    }
    else {
      if (!ecalIso)
        throw edm::Exception(edm::errors::Configuration, "ECAL PF cluster iso missing");
      outElectron.ecalIso = (*ecalIso)[inRef] - ecalIsoEA_.getEffectiveArea(scEta) * rhoCentralCalo;
      if (!hcalIso)
        throw edm::Exception(edm::errors::Configuration, "HCAL PF cluster iso missing");
      outElectron.hcalIso = (*hcalIso)[inRef] - hcalIsoEA_.getEffectiveArea(scEta) * rhoCentralCalo;
    }

    auto&& seedRef(sc.seed());
    if (seedRef.isNonnull()) {
      auto& seed(*seedRef);
      
      auto* seedHit(findHit(seed.seed()));
      if (seedHit)
        outElectron.eseed = seedHit->energy();
      else
        outElectron.eseed = 0.;
    }

    unsigned iPh(0);
    for (auto& photon : photons) {
      if (photon.superCluster() == scRef) {
        auto&& photonRef(photons.refAt(iPh));
        outElectron.chIsoPh = phCHIso[photonRef] - phCHIsoEA_.getEffectiveArea(scEta) * rho;
        outElectron.nhIsoPh = phNHIso[photonRef] - phNHIsoEA_.getEffectiveArea(scEta) * rho;
        outElectron.phIsoPh = phPhIso[photonRef] - phPhIsoEA_.getEffectiveArea(scEta) * rho;
      }
    }

    reco::CandidatePtr matchedPF(findPF(inElectron));

    if (matchedPF.isNonnull())
      outElectron.pfPt = matchedPF->pt();

    for (auto& smeared : inSmearedElectrons) {
      if (smeared.superCluster() == scRef) {
        outElectron.smearedPt = smeared.pt();
        break;
      }
    }

    for (auto& reg : inRegressionElectrons) {
      if (reg.superCluster() == scRef) {
        outElectron.regPt = reg.pt();
        break;
      }
    }

    // if (gsUnfixedElectrons && seedRef.isNonnull()) {
    //   auto eItr(gsFixMap.find(seedRef->seed().rawId()));
    //   if (eItr != gsFixMap.end())
    //     outElectron.originalPt = eItr->second->pt();
    // }
    // Following MET POG and doing the most simplistic match
    if (gsUnfixedElectrons) {
      for (auto& el : *gsUnfixedElectrons) {
        if (reco::deltaR(el, inElectron) < 0.01) {
          outElectron.originalPt = el.pt();
          break;
        }
      }
    }

    ptrList.push_back(inElectrons.ptrAt(iEl));
  }

  // sort the output electrons
  auto originalIndices(outElectrons.sort(panda::Particle::PtGreater));

  // make reco <-> panda mapping
  auto& eleEleMap(objectMap_->get<reco::GsfElectron, panda::Electron>());
  auto& scEleMap(objectMap_->get<reco::SuperCluster, panda::Electron>());
  auto& pfEleMap(objectMap_->get<reco::Candidate, panda::Electron>("pf"));
  auto& vtxEleMap(objectMap_->get<reco::Vertex, panda::Electron>());
  auto& genEleMap(objectMap_->get<reco::Candidate, panda::Electron>("gen"));
  
  for (unsigned iP(0); iP != outElectrons.size(); ++iP) {
    auto& outElectron(outElectrons[iP]);
    unsigned idx(originalIndices[iP]);
    eleEleMap.add(ptrList[idx], outElectron);
    scEleMap.add(edm::refToPtr(ptrList[idx]->superCluster()), outElectron);

    reco::CandidatePtr matchedPF(findPF(*ptrList[idx]));
    if (matchedPF.isNonnull()) {
      pfEleMap.add(matchedPF, outElectron);
      if (dynamic_cast<pat::PackedCandidate const*>(matchedPF.get())) {
        auto vtxRef(static_cast<pat::PackedCandidate const&>(*matchedPF).vertexRef());
        if (vtxRef.isNonnull())
          vtxEleMap.add(edm::refToPtr(vtxRef), outElectron);
      }
    }

    if (!isRealData_) {
      auto& inElectron(*ptrList[idx]);

      if (dynamic_cast<pat::Electron const*>(&inElectron)) {
        auto& patElectron(static_cast<pat::Electron const&>(inElectron));
        auto ref(patElectron.genParticleRef());
        if (ref.isNonnull())
          genEleMap.add(edm::refToPtr(ref), outElectron);
      }
    }
  }
}

void
ElectronsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  auto& scEleMap(objectMap_->get<reco::SuperCluster, panda::Electron>());
  auto& pfEleMap(objectMap_->get<reco::Candidate, panda::Electron>("pf"));
  auto& vtxEleMap(objectMap_->get<reco::Vertex, panda::Electron>());

  auto& scMap(_objectMaps.at("superClusters").get<reco::SuperCluster, panda::SuperCluster>().fwdMap);
  auto& vtxMap(_objectMaps.at("vertices").get<reco::Vertex, panda::RecoVertex>().fwdMap);

  for (auto& link : scEleMap.bwdMap) { // panda -> edm
    auto& outElectron(*link.first);
    auto& scPtr(link.second);

    outElectron.superCluster.setRef(scMap.at(scPtr));
  }

  if (_objectMaps.count("pfCandidates") != 0) {
    auto& pfMap(_objectMaps.at("pfCandidates").get<reco::Candidate, panda::PFCand>().fwdMap);

    for (auto& link : pfEleMap.bwdMap) { // panda -> edm
      auto& outElectron(*link.first);
      auto& pfPtr(link.second);

      outElectron.matchedPF.setRef(pfMap.at(pfPtr));
    }
  }

  for (auto& link : vtxEleMap.bwdMap) { // panda -> edm
    auto& outElectron(*link.first);
    auto& vtxPtr(link.second);

    outElectron.vertex.setRef(vtxMap.at(vtxPtr));
  }

  if (!isRealData_) {
    auto& genEleMap(objectMap_->get<reco::GenParticle, panda::Electron>("gen"));

    auto& genMap(_objectMaps.at("genParticles").get<reco::Candidate, panda::GenParticle>().fwdMap);

    for (auto& link : genEleMap.bwdMap) {
      auto& genPtr(link.second);
      if (genMap.find(genPtr) == genMap.end())
        continue;

      auto& outElectron(*link.first);
      outElectron.matchedGen.setRef(genMap.at(genPtr));
    }
  }

  if (useTrigger_) {
    auto& objMap(_objectMaps.at("global").get<pat::TriggerObjectStandAlone, VString>().fwdMap);

    std::vector<pat::TriggerObjectStandAlone const*> triggerObjects[panda::Electron::nTriggerObjects];

    // loop over the trigger filters we are interested in
    for (unsigned iT(0); iT != panda::Electron::nTriggerObjects; ++iT) {
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

    auto& eleEleMap(objectMap_->get<reco::GsfElectron, panda::Electron>().fwdMap);

    for (auto& link : eleEleMap) { // edm -> panda
      auto& inElectron(*link.first);
      auto& outElectron(*link.second);

      for (unsigned iT(0); iT != panda::Electron::nTriggerObjects; ++iT) {
        for (auto* obj : triggerObjects[iT]) {
          if (reco::deltaR(inElectron, *obj) < 0.3) {
            outElectron.triggerMatch[iT] = true;
            break;
          }
        }
      }
    }
  }
}

DEFINE_TREEFILLER(ElectronsFiller);
