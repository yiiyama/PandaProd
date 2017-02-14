#include "../interface/PhotonsFiller.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/Math/interface/deltaR.h"

#include <cmath>

PhotonsFiller::PhotonsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  chIsoEA_(getParameter_<edm::FileInPath>(_cfg, "chIsoEA").fullPath()),
  nhIsoEA_(getParameter_<edm::FileInPath>(_cfg, "nhIsoEA").fullPath()),
  phIsoEA_(getParameter_<edm::FileInPath>(_cfg, "phIsoEA").fullPath()),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.)),
  maxEta_(getParameter_<double>(_cfg, "maxEta", 10.))
{
  getToken_(photonsToken_, _cfg, _coll, "photons");
  getToken_(rawPhotonsToken_, _cfg, _coll, "rawPhotons");
  getToken_(regressionPhotonsToken_, _cfg, _coll, "regressionPhotons");
  getToken_(ebHitsToken_, _cfg, _coll, "common", "ebHits");
  getToken_(eeHitsToken_, _cfg, _coll, "common", "eeHits");
  getToken_(looseIdToken_, _cfg, _coll, "looseId");
  getToken_(mediumIdToken_, _cfg, _coll, "mediumId");
  getToken_(tightIdToken_, _cfg, _coll, "tightId");
  getToken_(chIsoToken_, _cfg, _coll, "chIso");
  getToken_(nhIsoToken_, _cfg, _coll, "nhIso");
  getToken_(phIsoToken_, _cfg, _coll, "phIso");
  getToken_(wchIsoToken_, _cfg, _coll, "wchIso");
  getToken_(rhoToken_, _cfg, _coll, "rho", "rho");
  if (!isRealData_)
    getToken_(genParticlesToken_, _cfg, _coll, "common", "finalStateParticles");

  if (useTrigger_) {
    for (unsigned iT(0); iT != panda::nPhotonTriggerObjects; ++iT) {
      std::string name(panda::PhotonTriggerObjectName[iT]); // "f<trigger filter name>"
      auto filters(getParameter_<VString>(_cfg, "triggerObjects." + name.substr(1)));
      triggerObjects_[iT].insert(filters.begin(), filters.end());
    }
  }

  chIsoLeakage_[0].Compile(getParameter_<std::string>(_cfg, "chIsoLeakage.EB", "").c_str());
  chIsoLeakage_[1].Compile(getParameter_<std::string>(_cfg, "chIsoLeakage.EE", "").c_str());
  nhIsoLeakage_[0].Compile(getParameter_<std::string>(_cfg, "nhIsoLeakage.EB", "").c_str());
  nhIsoLeakage_[1].Compile(getParameter_<std::string>(_cfg, "nhIsoLeakage.EE", "").c_str());
  phIsoLeakage_[0].Compile(getParameter_<std::string>(_cfg, "phIsoLeakage.EB", "").c_str());
  phIsoLeakage_[1].Compile(getParameter_<std::string>(_cfg, "phIsoLeakage.EE", "").c_str());
}

void
PhotonsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("photons");

  if (isRealData_)
    _eventBranches += {"!photons.geniso", "!photons.matchedGen_"};
  if (!useTrigger_)
    _eventBranches.emplace_back("!photons.triggerMatch");
}

void
PhotonsFiller::addOutput(TFile& _outputFile)
{
  TDirectory::TContext context(&_outputFile);
  TTree* t;
  t = panda::makePhotonTriggerObjectTree();
  t->Write();
  delete t;
}

void
PhotonsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inPhotons(getProduct_(_inEvent, photonsToken_));
  auto& inRawPhotons(getProduct_(_inEvent, rawPhotonsToken_));
  auto& inRegressionPhotons(getProduct_(_inEvent, regressionPhotonsToken_));
  auto& ebHits(getProduct_(_inEvent, ebHitsToken_));
  auto& eeHits(getProduct_(_inEvent, eeHitsToken_));
  auto& looseId(getProduct_(_inEvent, looseIdToken_));
  auto& mediumId(getProduct_(_inEvent, mediumIdToken_));
  auto& tightId(getProduct_(_inEvent, tightIdToken_));
  auto& chIso(getProduct_(_inEvent, chIsoToken_));
  auto& nhIso(getProduct_(_inEvent, nhIsoToken_));
  auto& phIso(getProduct_(_inEvent, phIsoToken_));
  auto& wchIso(getProduct_(_inEvent, wchIsoToken_));
  double rho(getProduct_(_inEvent, rhoToken_));
  // final state gen particles
  pat::PackedGenParticleCollection const* genParticles(0);
  if (!isRealData_)
    genParticles = &getProduct_(_inEvent, genParticlesToken_);

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

  noZS::EcalClusterLazyTools lazyTools(_inEvent, _setup, ebHitsToken_.second, eeHitsToken_.second);

  auto& outPhotons(_outEvent.photons);

  std::vector<edm::Ptr<reco::Photon>> ptrList;

  unsigned iPh(-1);
  for (auto& inPhoton : inPhotons) {
    ++iPh;
    if (inPhoton.pt() < minPt_)
      continue;
    if (inPhoton.eta() > maxEta_)
      continue;

    auto&& inRef(inPhotons.refAt(iPh));

    bool isPAT(dynamic_cast<pat::Photon const*>(&inPhoton));

    auto&& scRef(inPhoton.superCluster());
    auto& sc(*scRef);

    auto& outPhoton(outPhotons.create_back());

    fillP4(outPhoton, inPhoton);

    double scEta(std::abs(sc.eta()));

    unsigned iDet(scEta < 1.4442 ? 0 : 1);

    outPhoton.sieie = inPhoton.full5x5_sigmaIetaIeta();
    outPhoton.sipip = inPhoton.full5x5_showerShapeVariables().sigmaIphiIphi;
    outPhoton.hOverE = inPhoton.hadTowOverEm();
    outPhoton.pixelVeto = !inPhoton.hasPixelSeed();
    if (isPAT)
      outPhoton.csafeVeto = static_cast<pat::Photon const&>(inPhoton).passElectronVeto();

    outPhoton.chiso = chIso[inRef] - chIsoEA_.getEffectiveArea(scEta) * rho;
    if (chIsoLeakage_[iDet].IsValid())
      outPhoton.chiso -= chIsoLeakage_[iDet].Eval(outPhoton.pt());
    outPhoton.nhiso = nhIso[inRef] - nhIsoEA_.getEffectiveArea(scEta) * rho;
    if (nhIsoLeakage_[iDet].IsValid())
      outPhoton.nhiso -= nhIsoLeakage_[iDet].Eval(outPhoton.pt());
    outPhoton.phoiso = phIso[inRef] - phIsoEA_.getEffectiveArea(scEta) * rho;
    if (phIsoLeakage_[iDet].IsValid())
      outPhoton.phoiso -= phIsoLeakage_[iDet].Eval(outPhoton.pt());
    outPhoton.chisoWorst = wchIso[inRef];
    
    outPhoton.loose = looseId[inRef];
    outPhoton.medium = mediumId[inRef];
    outPhoton.tight = tightId[inRef];
    // Effective area hard-coded!!
    double highptEA(0.);
    if (scEta < 0.9)
      highptEA = 0.17;
    else if (scEta < 1.479)
      highptEA = 0.14;
    else if (scEta < 2.)
      highptEA = 0.11;
    else if (scEta < 2.2)
      highptEA = 0.14;
    else
      highptEA = 0.22;

    if (scEta < 1.479)
      outPhoton.highpt = outPhoton.hOverE < 0.05 &&
        outPhoton.sieie < 0.0105 &&
        chIso[inRef] < 5. &&
        phIso[inRef] + 0.0045 * outPhoton.pt() - highptEA * rho < 5.25;
    else if (scEta < 2.)
      outPhoton.highpt = outPhoton.hOverE < 0.05 &&
        outPhoton.sieie < 0.028 &&
        chIso[inRef] < 5. &&
        phIso[inRef] + 0.0045 * outPhoton.pt() - highptEA * rho < 4.5;
    else
      outPhoton.highpt = outPhoton.hOverE < 0.05 &&
        outPhoton.sieie < 0.028 &&
        chIso[inRef] < 5. &&
        phIso[inRef] + 0.003 * outPhoton.pt() - highptEA * rho < 4.5;

    outPhoton.mipEnergy = inPhoton.mipTotEnergy();

    outPhoton.e33 = inPhoton.e3x3();
    outPhoton.r9 = inPhoton.r9();

    outPhoton.etaWidth = sc.etaWidth();
    outPhoton.phiWidth = sc.phiWidth();

    auto&& seedRef(sc.seed());
    if (seedRef.isNonnull()) {
      auto& seed(*seedRef);
      outPhoton.emax = lazyTools.eMax(seed);
      outPhoton.e2nd = lazyTools.e2nd(seed);
      outPhoton.e4 = lazyTools.eTop(seed) + lazyTools.eRight(seed) + lazyTools.eBottom(seed) + lazyTools.eLeft(seed);
      
      auto* seedHit(findHit(seed.seed()));
      if (seedHit) {
        outPhoton.time = seedHit->time();
        outPhoton.eseed = seedHit->energy();
      }
      else {
        outPhoton.time = 0.;
        outPhoton.eseed = 0.;
      }
    }

    outPhoton.timeSpan = 0.;    
    for (auto& hf : sc.hitsAndFractions()) {
      auto* hit(findHit(hf.first));
      if (!hit || hit->energy() < 1.)
        continue;

      double dt(outPhoton.time - hit->time());
      if (std::abs(dt) > std::abs(outPhoton.timeSpan))
        outPhoton.timeSpan = dt;
    }

    if (!isRealData_) {
      // compute geniso

      if (isPAT) {
        auto& patPhoton(static_cast<pat::Photon const&>(inPhoton));

        auto* gen(patPhoton.genPhoton());
        if (gen && gen->mother()) {
          auto* mother(gen->mother());
          
          // look for the final state particle that matches gen
          pat::PackedGenParticle const* matched(0);

          for (auto& fs : *genParticles) {
            if (fs.mother(0) != mother)
              continue;

            if (fs.pdgId() != gen->pdgId())
              continue;

            // arbitrary cuts..
            if (reco::deltaR(fs, *gen) < 0.01 && std::abs(fs.pt() - gen->pt()) < 0.1) {
              matched = &fs;
              break;
            }
          }

          if (matched) {
            for (auto& fs : *genParticles) {
              if (reco::deltaR(fs, *matched) < 0.3) {
                outPhoton.geniso += fs.pt();
              }
            }
          }
        }
      }      
    }

    for (auto& raw : inRawPhotons) {
      if (raw.superCluster() == scRef) {
        outPhoton.rawPt = raw.pt();
        break;
      }
    }

    for (auto& reg : inRegressionPhotons) {
      if (reg.superCluster() == scRef) {
        outPhoton.regPt = reg.pt();
        break;
      }
    }

    ptrList.push_back(inPhotons.ptrAt(iPh));
  }

  auto originalIndices(outPhotons.sort(panda::Particle::PtGreater));

  // make reco <-> panda mapping
  auto& phoPhoMap(objectMap_->get<reco::Photon, panda::Photon>());
  auto& scPhoMap(objectMap_->get<reco::SuperCluster, panda::Photon>());
  auto& genPhoMap(objectMap_->get<reco::GenParticle, panda::Photon>());
  
  for (unsigned iP(0); iP != outPhotons.size(); ++iP) {
    auto& outPhoton(outPhotons[iP]);
    unsigned idx(originalIndices[iP]);
    phoPhoMap.add(ptrList[idx], outPhoton);
    scPhoMap.add(edm::refToPtr(ptrList[idx]->superCluster()), outPhoton);

    if (!isRealData_) {
      auto& inPhoton(*ptrList[idx]);

      if (dynamic_cast<pat::Photon const*>(&inPhoton)) {
        auto& patPhoton(static_cast<pat::Photon const&>(inPhoton));
        // PAT gen matching - will match to a photon even in case of a lepton FSR
        auto ref(patPhoton.genParticleRef());
        if (ref.isNonnull())
          genPhoMap.add(edm::refToPtr(ref), outPhoton);
      }
    }
  }
}

void
PhotonsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  auto& scPhoMap(objectMap_->get<reco::SuperCluster, panda::Photon>().bwdMap);

  auto& scMap(_objectMaps.at("superClusters").get<reco::SuperCluster, panda::SuperCluster>().fwdMap);

  for (auto& link : scPhoMap) { // panda -> edm
    auto& outPhoton(*link.first);
    auto& scPtr(link.second);

    outPhoton.superCluster.setRef(scMap.at(scPtr));
  }

  if (!isRealData_) {
    auto& genPhoMap(objectMap_->get<reco::GenParticle, panda::Photon>());

    auto& genMap(_objectMaps.at("genParticles").get<reco::GenParticle, panda::GenParticle>().fwdMap);

    for (auto& link : genPhoMap.bwdMap) {
      auto& genPtr(link.second);
      if (genMap.find(genPtr) == genMap.end())
        continue;

      auto& outPhoton(*link.first);
      outPhoton.matchedGen.setRef(genMap.at(genPtr));
    }
  }

  if (useTrigger_) {
    auto& objMap(_objectMaps.at("global").get<pat::TriggerObjectStandAlone, VString>().fwdMap);

    std::vector<pat::TriggerObjectStandAlone const*> triggerObjects[panda::nPhotonTriggerObjects];

    // loop over the trigger filters we are interested in
    for (unsigned iT(0); iT != panda::nPhotonTriggerObjects; ++iT) {
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

    auto& phoPhoMap(objectMap_->get<reco::Photon, panda::Photon>().fwdMap);

    for (auto& link : phoPhoMap) { // edm -> panda
      auto& inPhoton(*link.first);
      auto& outPhoton(*link.second);

      for (unsigned iT(0); iT != panda::nPhotonTriggerObjects; ++iT) {
        for (auto* obj : triggerObjects[iT]) {
          if (reco::deltaR(inPhoton, *obj) < 0.3) {
            outPhoton.triggerMatch[iT] = true;
            break;
          }
        }
      }
    }
  }
}

DEFINE_TREEFILLER(PhotonsFiller);
