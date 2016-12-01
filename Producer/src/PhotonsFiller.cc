#include "../interface/PhotonsFiller.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/Math/interface/deltaR.h"

#include <cmath>

PhotonsFiller::PhotonsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name),
  chIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name).getUntrackedParameter<edm::FileInPath>("chIsoEA").fullPath()),
  nhIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name).getUntrackedParameter<edm::FileInPath>("nhIsoEA").fullPath()),
  phIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name).getUntrackedParameter<edm::FileInPath>("phIsoEA").fullPath()),
  useTrigger_(_cfg.getUntrackedParameter<bool>("useTrigger"))
{
  auto& fillerCfg(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name));

  getToken_(photonsToken_, _cfg, _coll, "photons");
  getToken_(ebHitsToken_, _cfg, _coll, "ebHits");
  getToken_(eeHitsToken_, _cfg, _coll, "eeHits");
  //  getToken_(electronsToken_, _cfg, _coll, "electrons");
  getToken_(looseIdToken_, fillerCfg, _coll, "looseId");
  getToken_(mediumIdToken_, fillerCfg, _coll, "mediumId");
  getToken_(tightIdToken_, fillerCfg, _coll, "tightId");
  getToken_(chIsoToken_, fillerCfg, _coll, "chIso");
  getToken_(nhIsoToken_, fillerCfg, _coll, "nhIso");
  getToken_(phIsoToken_, fillerCfg, _coll, "phIso");
  getToken_(wchIsoToken_, fillerCfg, _coll, "wchIso");
  getToken_(rhoToken_, _cfg, _coll, "rho");
  if (useTrigger_) {
    getToken_(triggerObjectsToken_, _cfg, _coll, "triggerObjects");
    l1Filters_ = fillerCfg.getUntrackedParameter<VString>("l1Filters");
    hltFilters_ = fillerCfg.getUntrackedParameter<VString>("hltFilters");
    if (l1Filters_.size() != panda::nPhotonL1Objects)
      throw edm::Exception(edm::errors::Configuration, "PhotonsFiller")
        << "photonL1Filters.size()";
    if (hltFilters_.size() != panda::nPhotonHLTObjects)
      throw edm::Exception(edm::errors::Configuration, "PhotonsFiller")
        << "photonHLTFilters.size()";
  }

  if (fillerCfg.exists("chIsoLeakage")) {
    auto& cfg(fillerCfg.getUntrackedParameterSet("chIsoLeakage"));
    if (cfg.exists("EB"))
      chIsoLeakage_[0].Compile(cfg.getUntrackedParameter<std::string>("EB").c_str());
    if (cfg.exists("EE"))
      chIsoLeakage_[1].Compile(cfg.getUntrackedParameter<std::string>("EE").c_str());
  }
  if (fillerCfg.exists("nhIsoLeakage")) {
    auto& cfg(fillerCfg.getUntrackedParameterSet("nhIsoLeakage"));
    if (cfg.exists("EB"))
      nhIsoLeakage_[0].Compile(cfg.getUntrackedParameter<std::string>("EB").c_str());
    if (cfg.exists("EE"))
      nhIsoLeakage_[1].Compile(cfg.getUntrackedParameter<std::string>("EE").c_str());
  }
  if (fillerCfg.exists("phIsoLeakage")) {
    auto& cfg(fillerCfg.getUntrackedParameterSet("phIsoLeakage"));
    if (cfg.exists("EB"))
      phIsoLeakage_[0].Compile(cfg.getUntrackedParameter<std::string>("EB").c_str());
    if (cfg.exists("EE"))
      phIsoLeakage_[1].Compile(cfg.getUntrackedParameter<std::string>("EE").c_str());
  }

  minPt_ = fillerCfg.getUntrackedParameter<double>("minPt", -1.);
  maxEta_ = fillerCfg.getUntrackedParameter<double>("maxEta", 10.);
}

void
PhotonsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup, ObjectMapStore& _objectMaps)
{
  auto& inPhotons(getProduct_(_inEvent, photonsToken_, "photons"));
  auto& ebHits(getProduct_(_inEvent, ebHitsToken_, "ebHits"));
  auto& eeHits(getProduct_(_inEvent, eeHitsToken_, "eeHits"));
  //  auto& electrons(getProduct_(_inEvent, electronsToken_, "electrons"));
  auto& looseId(getProduct_(_inEvent, looseIdToken_, "photonLooseId"));
  auto& mediumId(getProduct_(_inEvent, mediumIdToken_, "photonMediumId"));
  auto& tightId(getProduct_(_inEvent, tightIdToken_, "photonTightId"));
  auto& chIso(getProduct_(_inEvent, chIsoToken_, "chIso"));
  auto& nhIso(getProduct_(_inEvent, nhIsoToken_, "nhIso"));
  auto& phIso(getProduct_(_inEvent, phIsoToken_, "phIso"));
  auto& wchIso(getProduct_(_inEvent, wchIsoToken_, "wchIso"));
  double rho(getProduct_(_inEvent, rhoToken_, "rho"));

  auto findHit([&ebHits, &eeHits](DetId const& id)->EcalRecHit const& {
      EcalRecHitCollection const* hits(0);
      if (id.subdetId() == EcalBarrel)
        hits = &ebHits;
      else
        hits = &eeHits;

      auto&& hitItr(hits->find(id));
      if (hitItr == hits->end())
        throw std::logic_error("Ecal rec hit not found");

      return *hitItr;
    });

  noZS::EcalClusterLazyTools lazyTools(_inEvent, _setup, ebHitsToken_, eeHitsToken_);

  std::vector<pat::TriggerObjectStandAlone const*> l1Objects[panda::nPhotonL1Objects];
  std::vector<pat::TriggerObjectStandAlone const*> hltObjects[panda::nPhotonHLTObjects];
  if (useTrigger_) {
    auto& triggerObjects(getProduct_(_inEvent, triggerObjectsToken_, "triggerObjects"));
    for (auto& obj : triggerObjects) {
      for (unsigned iF(0); iF != panda::nPhotonL1Objects; ++iF) {
        if (obj.hasFilterLabel(l1Filters_[iF]))
          l1Objects[iF].push_back(&obj);
      }
      for (unsigned iF(0); iF != panda::nPhotonHLTObjects; ++iF) {
        if (obj.hasFilterLabel(hltFilters_[iF]))
          hltObjects[iF].push_back(&obj);
      }
    }
  }

  auto& outPhotons(_outEvent.photons);

  std::vector<edm::Ptr<reco::Photon>> ptrList;
  std::vector<edm::Ptr<reco::SuperCluster>> scPtrList;

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

    outPhoton.scRawPt = sc.rawEnergy() / std::cosh(scEta);

    outPhoton.isEB = scEta < 1.4442;
    unsigned iDet(outPhoton.isEB ? 0 : 1);

    outPhoton.sieie = inPhoton.full5x5_sigmaIetaIeta();
    outPhoton.sipip = inPhoton.full5x5_showerShapeVariables().sigmaIphiIphi;
    outPhoton.hOverE = inPhoton.hadTowOverEm();
    outPhoton.pixelVeto = !inPhoton.hasPixelSeed();
    if (isPAT)
      outPhoton.csafeVeto = static_cast<pat::Photon const&>(inPhoton).passElectronVeto();

    outPhoton.chiso = chIso[inRef] - chIsoEA_.getEffectiveArea(scEta) * rho;
    if (chIsoLeakage_[iDet].IsValid())
      outPhoton.chiso -= chIsoLeakage_[iDet].Eval(outPhoton.pt);
    outPhoton.nhiso = nhIso[inRef] - nhIsoEA_.getEffectiveArea(scEta) * rho;
    if (nhIsoLeakage_[iDet].IsValid())
      outPhoton.nhiso -= nhIsoLeakage_[iDet].Eval(outPhoton.pt);
    outPhoton.phoiso = phIso[inRef] - phIsoEA_.getEffectiveArea(scEta) * rho;
    if (phIsoLeakage_[iDet].IsValid())
      outPhoton.phoiso -= phIsoLeakage_[iDet].Eval(outPhoton.pt);
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
        phIso[inRef] + 0.0045 * outPhoton.pt - highptEA * rho < 5.25;
    else if (scEta < 2.)
      outPhoton.highpt = outPhoton.hOverE < 0.05 &&
        outPhoton.sieie < 0.028 &&
        chIso[inRef] < 5. &&
        phIso[inRef] + 0.0045 * outPhoton.pt - highptEA * rho < 4.5;
    else
      outPhoton.highpt = outPhoton.hOverE < 0.05 &&
        outPhoton.sieie < 0.028 &&
        chIso[inRef] < 5. &&
        phIso[inRef] + 0.003 * outPhoton.pt - highptEA * rho < 4.5;

    outPhoton.mipEnergy = inPhoton.mipTotEnergy();

    outPhoton.e33 = inPhoton.e3x3();
    outPhoton.r9 = inPhoton.r9();

    outPhoton.etaWidth = sc.etaWidth();
    outPhoton.phiWidth = sc.phiWidth();

    outPhoton.timeSpan = 0.;    
    for (auto& hf : sc.hitsAndFractions()) {
      auto& hit(findHit(hf.first));
      if (hit.energy() < 1.)
        continue;

      double dt(outPhoton.time - hit.time());
      if (std::abs(dt) > std::abs(outPhoton.timeSpan))
        outPhoton.timeSpan = dt;
    }

    auto&& seedRef(sc.seed());
    if (seedRef.isNonnull()) {
      auto& seed(*seedRef);
      outPhoton.emax = lazyTools.eMax(seed);
      outPhoton.e2nd = lazyTools.e2nd(seed);
      outPhoton.e4 = lazyTools.eTop(seed) + lazyTools.eRight(seed) + lazyTools.eBottom(seed) + lazyTools.eLeft(seed);
      
      auto& seedHit(findHit(seed.hitsAndFractions()[0].first));
      outPhoton.time = seedHit.time();
    }

    if (useTrigger_) {
      for (unsigned iF(0); iF != panda::nPhotonL1Objects; ++iF) {
        for (auto* obj : l1Objects[iF]) {
          if (reco::deltaR(inPhoton, *obj) < 0.3) {
            outPhoton.matchL1[iF] = true;
            break;
          }
        }
      }
      for (unsigned iF(0); iF != panda::nPhotonHLTObjects; ++iF) {
        for (auto* obj : hltObjects[iF]) {
          if (reco::deltaR(inPhoton, *obj) < 0.3) {
            outPhoton.matchHLT[iF] = true;
            break;
          }
        }
      }
    }

    outPhoton.genMatchDR = -1.;
    outPhoton.matchedGen = 0;
    outPhoton.genIso = 0.;

    if (!_inEvent.isRealData()) {
      // TODO
    }

    ptrList.push_back(inPhotons.ptrAt(iPh));
    scPtrList.push_back(edm::refToPtr(scRef));
  }

  auto originalIndices(outPhotons.sort(panda::ptGreater));

  // make reco <-> panda mapping

  auto& objectMap(_objectMaps.get<reco::Photon, panda::PPhoton>("photons"));
  scPtrs_.clear();
  
  for (unsigned iP(0); iP != outPhotons.size(); ++iP) {
    auto& outPhoton(outPhotons[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outPhoton);
    scPtrs_.emplace_back(&outPhoton, scPtrList[idx]);
  }
}

void
PhotonsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  auto& scMap(_objectMaps.get<reco::SuperCluster, panda::PSuperCluster>("superClusters"));
  for (auto& link : scPtrs_) {
    auto& outPhoton(*link.first);
    auto& scPtr(link.second);

    outPhoton.superCluster = scMap.fwdMap.at(scPtr);
  }
}

DEFINE_TREEFILLER(PhotonsFiller);
