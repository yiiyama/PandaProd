#include "../interface/ElectronsFiller.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/Math/interface/deltaR.h"

#include <cmath>

ElectronsFiller::ElectronsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name),
  combIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name).getUntrackedParameter<edm::FileInPath>("combIsoEA").fullPath()),
  ecalIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name).getUntrackedParameter<edm::FileInPath>("electronEcalIsoEA").fullPath()),
  hcalIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name).getUntrackedParameter<edm::FileInPath>("electronHcalIsoEA").fullPath()),
  phCHIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet("photons").getUntrackedParameter<edm::FileInPath>("chIsoEA").fullPath()),
  phNHIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet("photons").getUntrackedParameter<edm::FileInPath>("nhIsoEA").fullPath()),
  phPhIsoEA_(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet("photons").getUntrackedParameter<edm::FileInPath>("phIsoEA").fullPath()),
  useTrigger_(_cfg.getUntrackedParameter<bool>("useTrigger"))
{
  auto& fillerCfg(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name));
  auto& photonCfg(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet("photons"));

  getToken_(electronsToken_, _cfg, _coll, "electrons");
  getToken_(photonsToken_, _cfg, _coll, "photons");
  getToken_(vetoIdToken_, fillerCfg, _coll, "vetoId");
  getToken_(looseIdToken_, fillerCfg, _coll, "looseId");
  getToken_(mediumIdToken_, fillerCfg, _coll, "mediumId");
  getToken_(tightIdToken_, fillerCfg, _coll, "tightId");
  getToken_(phCHIsoToken_, photonCfg, _coll, "chIso");
  getToken_(phNHIsoToken_, photonCfg, _coll, "nhIso");
  getToken_(phPhIsoToken_, photonCfg, _coll, "phIso");
  getToken_(ecalIsoToken_, fillerCfg, _coll, "ecalIso");
  getToken_(hcalIsoToken_, fillerCfg, _coll, "hcalIso");
  getToken_(rhoToken_, _cfg, _coll, "rho");
  if (useTrigger_) {
    getToken_(triggerObjectsToken_, _cfg, _coll, "triggerObjects");
    hltFilters_ = fillerCfg.getUntrackedParameter<VString>("hltFilters");
    if (hltFilters_.size() != panda::nElectronHLTObjects)
      throw edm::Exception(edm::errors::Configuration, "ElectronsFiller")
        << "electronHLTFilters.size()";
  }

  minPt_ = fillerCfg.getUntrackedParameter<double>("minPt", -1.);
  maxEta_ = fillerCfg.getUntrackedParameter<double>("maxEta", 10.);
}

void
ElectronsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup, ObjectMapStore& _objectMaps)
{
  auto& inElectrons(getProduct_(_inEvent, electronsToken_, "electrons"));
  auto& photons(getProduct_(_inEvent, photonsToken_, "photons"));
  auto& vetoId(getProduct_(_inEvent, vetoIdToken_, "vetoId"));
  auto& looseId(getProduct_(_inEvent, looseIdToken_, "looseId"));
  auto& mediumId(getProduct_(_inEvent, mediumIdToken_, "mediumId"));
  auto& tightId(getProduct_(_inEvent, tightIdToken_, "tightId"));
  auto& phCHIso(getProduct_(_inEvent, phCHIsoToken_, "chIso"));
  auto& phNHIso(getProduct_(_inEvent, phNHIsoToken_, "nhIso"));
  auto& phPhIso(getProduct_(_inEvent, phPhIsoToken_, "phIso"));
  auto& ecalIso(getProduct_(_inEvent, ecalIsoToken_, "ecalIso"));
  auto& hcalIso(getProduct_(_inEvent, hcalIsoToken_, "hcalIso"));
  double rho(getProduct_(_inEvent, rhoToken_, "rho"));

  std::vector<pat::TriggerObjectStandAlone const*> hltObjects[panda::nElectronHLTObjects];
  if (useTrigger_) {
    auto& triggerObjects(getProduct_(_inEvent, triggerObjectsToken_, "triggerObjects"));
    for (auto& obj : triggerObjects) {
      for (unsigned iF(0); iF != panda::nElectronHLTObjects; ++iF) {
        if (obj.hasFilterLabel(hltFilters_[iF]))
          hltObjects[iF].push_back(&obj);
      }
    }
  }

  auto& outElectrons(_outEvent.electrons);

  std::vector<edm::Ptr<reco::GsfElectron>> ptrList;
  std::vector<edm::Ptr<reco::SuperCluster>> scPtrList;

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

    outElectron.q = inElectron.charge();

    outElectron.sieie = inElectron.full5x5_sigmaIetaIeta();
    outElectron.sipip = inElectron.full5x5_sigmaIphiIphi();
    outElectron.hOverE = inElectron.hadronicOverEm();

    double scEta(std::abs(sc.eta()));

    auto& pfIso(inElectron.pfIsolationVariables());
    outElectron.chiso = pfIso.sumChargedHadronPt;
    outElectron.nhiso = pfIso.sumNeutralHadronEt;
    outElectron.phoiso = pfIso.sumPhotonEt;
    outElectron.puiso = pfIso.sumPUPt;
    outElectron.isoPUOffset = combIsoEA_.getEffectiveArea(scEta) * rho;
    outElectron.ecaliso = ecalIso[inRef] - ecalIsoEA_.getEffectiveArea(scEta) * rho;
    outElectron.hcaliso = hcalIso[inRef] - hcalIsoEA_.getEffectiveArea(scEta) * rho;

    unsigned iPh(0);
    for (auto& photon : photons) {
      if (photon.superCluster() == scRef) {
        auto&& photonRef(photons.refAt(iPh));
        outElectron.chisoPh = phCHIso[photonRef] - phCHIsoEA_.getEffectiveArea(scEta) * rho;
        outElectron.nhisoPh = phNHIso[photonRef] - phNHIsoEA_.getEffectiveArea(scEta) * rho;
        outElectron.phisoPh = phPhIso[photonRef] - phPhIsoEA_.getEffectiveArea(scEta) * rho;
      }
    }

    if (useTrigger_) {
      for (unsigned iF(0); iF != panda::nElectronHLTObjects; ++iF) {
        for (auto* obj : hltObjects[iF]) {
          if (reco::deltaR(inElectron, *obj) < 0.3) {
            outElectron.matchHLT[iF] = true;
            break;
          }
        }
      }
    }

    if (!_inEvent.isRealData()) {
      outElectron.tauDecay = false;
      outElectron.hadDecay = false;
    }

    ptrList.push_back(inElectrons.ptrAt(iEl));
    scPtrList.push_back(edm::refToPtr(scRef));
  }

  // sort the output electrons
  auto originalIndices(outElectrons.sort(panda::ptGreater));

  // make reco <-> panda mapping

  auto& objectMap(_objectMaps.get<reco::GsfElectron, panda::PElectron>("electrons"));
  scPtrs_.clear();
  
  for (unsigned iP(0); iP != outElectrons.size(); ++iP) {
    auto& outElectron(outElectrons[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outElectron);
    scPtrs_.emplace_back(&outElectron, scPtrList[idx]);
  }
}

void
ElectronsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  auto& scMap(_objectMaps.get<reco::SuperCluster, panda::PSuperCluster>("superClusters"));
  for (auto& link : scPtrs_) {
    auto& outElectron(*link.first);
    auto& scPtr(link.second);

    outElectron.superCluster = scMap.fwdMap.at(scPtr);
  }
}

DEFINE_TREEFILLER(ElectronsFiller);
