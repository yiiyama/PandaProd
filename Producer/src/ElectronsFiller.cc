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
  FillerBase(_name, _cfg),
  combIsoEA_(getParameter_<edm::FileInPath>(_cfg, "combIsoEA").fullPath()),
  ecalIsoEA_(getParameter_<edm::FileInPath>(_cfg, "ecalIsoEA").fullPath()),
  hcalIsoEA_(getParameter_<edm::FileInPath>(_cfg, "hcalIsoEA").fullPath()),
  phCHIsoEA_(getFillerParameter_<edm::FileInPath>(_cfg, "photons", "chIsoEA").fullPath()),
  phNHIsoEA_(getFillerParameter_<edm::FileInPath>(_cfg, "photons", "nhIsoEA").fullPath()),
  phPhIsoEA_(getFillerParameter_<edm::FileInPath>(_cfg, "photons", "phIsoEA").fullPath())
{
  getToken_(electronsToken_, _cfg, _coll, "electrons");
  getToken_(photonsToken_, _cfg, _coll, "photons", "photons");
  getToken_(vetoIdToken_, _cfg, _coll, "vetoId");
  getToken_(looseIdToken_, _cfg, _coll, "looseId");
  getToken_(mediumIdToken_, _cfg, _coll, "mediumId");
  getToken_(tightIdToken_, _cfg, _coll, "tightId");
  getToken_(phCHIsoToken_, _cfg, _coll, "photons", "chIso");
  getToken_(phNHIsoToken_, _cfg, _coll, "photons", "nhIso");
  getToken_(phPhIsoToken_, _cfg, _coll, "photons", "phIso");
  getToken_(ecalIsoToken_, _cfg, _coll, "ecalIso");
  getToken_(hcalIsoToken_, _cfg, _coll, "hcalIso");
  getToken_(rhoToken_, _cfg, _coll, "rho", "rho");
  getToken_(rhoCentralCaloToken_, _cfg, _coll, "rho", "rhoCentralCalo");
  if (useTrigger_) {
    getToken_(triggerObjectsToken_, _cfg, _coll, "common", "triggerObjects");
    hltFilters_ = getParameter_<VString>(_cfg, "hltFilters");
    if (hltFilters_.size() != panda::nElectronHLTObjects)
      throw edm::Exception(edm::errors::Configuration, "ElectronsFiller")
        << "electronHLTFilters.size()";
  }

  minPt_ = getParameter_<double>(_cfg, "minPt", -1.);
  maxEta_ = getParameter_<double>(_cfg, "maxEta", 10.);
}

void
ElectronsFiller::addOutput(TFile& _outputFile)
{
  TDirectory::TContext(&_outputFile);
  auto* t(panda::makeElectronHLTObjectTree());
  t->Write();
  delete t;
}

void
ElectronsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  if (isRealData_) {
    char const* genBranches[] = {"!electrons.tauDecay", "!electrons.hadDecay", "!electrons.matchedGen_"};
    _eventBranches.insert(_eventBranches.end(), genBranches, genBranches + sizeof(genBranches));
  }
  if (!useTrigger_) {
    _eventBranches.push_back("!electrons.matchHLT");
  }
}

void
ElectronsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inElectrons(getProduct_(_inEvent, electronsToken_));
  auto& photons(getProduct_(_inEvent, photonsToken_));
  auto& vetoId(getProduct_(_inEvent, vetoIdToken_));
  auto& looseId(getProduct_(_inEvent, looseIdToken_));
  auto& mediumId(getProduct_(_inEvent, mediumIdToken_));
  auto& tightId(getProduct_(_inEvent, tightIdToken_));
  auto& phCHIso(getProduct_(_inEvent, phCHIsoToken_));
  auto& phNHIso(getProduct_(_inEvent, phNHIsoToken_));
  auto& phPhIso(getProduct_(_inEvent, phPhIsoToken_));
  auto& ecalIso(getProduct_(_inEvent, ecalIsoToken_));
  auto& hcalIso(getProduct_(_inEvent, hcalIsoToken_));
  double rho(getProduct_(_inEvent, rhoToken_));
  double rhoCentralCalo(getProduct_(_inEvent, rhoCentralCaloToken_));

  std::vector<pat::TriggerObjectStandAlone const*> hltObjects[panda::nElectronHLTObjects];
  if (useTrigger_) {
    auto& triggerObjects(getProduct_(_inEvent, triggerObjectsToken_));
    for (auto& obj : triggerObjects) {
      for (unsigned iF(0); iF != panda::nElectronHLTObjects; ++iF) {
        if (obj.hasFilterLabel(hltFilters_[iF]))
          hltObjects[iF].push_back(&obj);
      }
    }
  }

  auto& outElectrons(_outEvent.electrons);

  std::vector<edm::Ptr<reco::GsfElectron>> ptrList;

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
    outElectron.ecaliso = ecalIso[inRef] - ecalIsoEA_.getEffectiveArea(scEta) * rhoCentralCalo;
    outElectron.hcaliso = hcalIso[inRef] - hcalIsoEA_.getEffectiveArea(scEta) * rhoCentralCalo;

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
  }

  // sort the output electrons
  auto originalIndices(outElectrons.sort(panda::ptGreater));

  // make reco <-> panda mapping
  auto& eleEleMap(objectMap_->get<reco::GsfElectron, panda::PElectron>());
  auto& scEleMap(objectMap_->get<reco::SuperCluster, panda::PElectron>());
  
  for (unsigned iP(0); iP != outElectrons.size(); ++iP) {
    auto& outElectron(outElectrons[iP]);
    unsigned idx(originalIndices[iP]);
    eleEleMap.add(ptrList[idx], outElectron);
    scEleMap.add(edm::refToPtr(ptrList[idx]->superCluster()), outElectron);
  }
}

void
ElectronsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  auto& scEleMap(objectMap_->get<reco::SuperCluster, panda::PElectron>());

  auto& scMap(_objectMaps.at("superClusters").get<reco::SuperCluster, panda::PSuperCluster>().fwdMap);

  for (auto& link : scEleMap.bwdMap) { // panda -> edm
    auto& outElectron(*link.first);
    auto& scPtr(link.second);

    outElectron.superCluster = scMap.at(scPtr);
  }
}

DEFINE_TREEFILLER(ElectronsFiller);
