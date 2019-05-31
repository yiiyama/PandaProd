#include "../interface/ElectronsFiller.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "RecoEgamma/EgammaTools/interface/ConversionTools.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/Math/interface/deltaR.h"

#include <cmath>

ElectronsFiller::ElectronsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  vetoIdName_(getParameter_<std::string>(_cfg, "vetoId")),
  looseIdName_(getParameter_<std::string>(_cfg, "looseId")),
  mediumIdName_(getParameter_<std::string>(_cfg, "mediumId")),
  tightIdName_(getParameter_<std::string>(_cfg, "tightId")),
  hltIdName_(getParameter_<std::string>(_cfg, "hltId", "")),
  mvaWP90Name_(getParameter_<std::string>(_cfg, "mvaWP90", "")),
  mvaWP80Name_(getParameter_<std::string>(_cfg, "mvaWP80", "")),
  mvaWPLooseName_(getParameter_<std::string>(_cfg, "mvaWPLoose", "")),
  mvaIsoWP90Name_(getParameter_<std::string>(_cfg, "mvaIsoWP90", "")),
  mvaIsoWP80Name_(getParameter_<std::string>(_cfg, "mvaIsoWP80", "")),
  mvaIsoWPLooseName_(getParameter_<std::string>(_cfg, "mvaIsoWPLoose", "")),
  mvaValuesName_(getParameter_<std::string>(_cfg, "mvaValues", "")),
  // mvaCategoriesName_(getParameter_<std::string>(_cfg, "mvaCategories", "")),
  combIsoEA_(edm::FileInPath(getParameter_<std::string>(_cfg, "combIsoEA")).fullPath()),
  ecalIsoEA_(edm::FileInPath(getParameter_<std::string>(_cfg, "ecalIsoEA")).fullPath()),
  hcalIsoEA_(edm::FileInPath(getParameter_<std::string>(_cfg, "hcalIsoEA")).fullPath()),
  fillCorrectedPts_(getParameter_<bool>(_cfg, "fillCorrectedPts"))
{
  getToken_(electronsToken_, _cfg, _coll, "electrons");
  getToken_(photonsToken_, _cfg, _coll, "photons", "photons");
  getToken_(conversionsToken_, _cfg, _coll, "common", "conversions");
  getToken_(pfCandidatesToken_, _cfg, _coll, "common", "pfCandidates");
  getToken_(verticesToken_, _cfg, _coll, "common", "vertices");
  getToken_(ebHitsToken_, _cfg, _coll, "common", "ebHits");
  getToken_(eeHitsToken_, _cfg, _coll, "common", "eeHits");
  getToken_(beamSpotToken_, _cfg, _coll, "common", "beamSpot");
  getToken_(rhoToken_, _cfg, _coll, "rho", "rho");
  getToken_(rhoCentralCaloToken_, _cfg, _coll, "rho", "rhoCentralCalo");
  getToken_(ecalIsoToken_, _cfg, _coll, "ecalIso", false);
  getToken_(hcalIsoToken_, _cfg, _coll, "hcalIso", false);
}

void
ElectronsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("electrons");

  if (isRealData_)
    _eventBranches.emplace_back("!electrons.matchedGen_");
}

void
ElectronsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inElectrons(getProduct_(_inEvent, electronsToken_));
  auto& photons(getProduct_(_inEvent, photonsToken_));
  auto& pfCandidates(getProduct_(_inEvent, pfCandidatesToken_));
  auto& vertices(getProduct_(_inEvent, verticesToken_));
  auto& ebHits(getProduct_(_inEvent, ebHitsToken_));
  auto& eeHits(getProduct_(_inEvent, eeHitsToken_));
  auto& beamSpot(getProduct_(_inEvent, beamSpotToken_));
  auto* ecalIso(getProductSafe_(_inEvent, ecalIsoToken_));
  auto* hcalIso(getProductSafe_(_inEvent, hcalIsoToken_));
  double rho(getProduct_(_inEvent, rhoToken_));
  double rhoCentralCalo(getProduct_(_inEvent, rhoCentralCaloToken_));

  edm::Handle<reco::ConversionCollection> conversionsHandle;
  getProduct_(_inEvent, conversionsToken_, &conversionsHandle);

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

  unsigned iEl(-1);
  for (auto& inElectron : inElectrons) {
    ++iEl;
    auto&& inRef(inElectrons.refAt(iEl));
    auto&& scRef(inElectron.superCluster());
    auto& sc(*scRef);

    bool isPAT(dynamic_cast<pat::Electron const*>(&inElectron) != nullptr);

    auto& outElectron(outElectrons.create_back());

    fillP4(outElectron, inElectron);

    if (isPAT) {
      // We have now abandoned saving ID bits when running on AOD, but in practice we never do that
      auto& patElectron(static_cast<pat::Electron const&>(inElectron));

      auto idBit([&patElectron](std::string const& name)->bool {
          if (name.empty())
            return false;
          else
            return patElectron.electronID(name) > 0.;
        });

      outElectron.veto = idBit(vetoIdName_);
      outElectron.loose = idBit(looseIdName_);
      outElectron.medium = idBit(mediumIdName_);
      outElectron.tight = idBit(tightIdName_);
      outElectron.hltsafe = idBit(hltIdName_);
      outElectron.mvaWP90 = idBit(mvaWP90Name_);
      outElectron.mvaWP80 = idBit(mvaWP80Name_);
      outElectron.mvaWPLoose = idBit(mvaWPLooseName_);
      outElectron.mvaIsoWP90 = idBit(mvaIsoWP90Name_);
      outElectron.mvaIsoWP80 = idBit(mvaIsoWP80Name_);
      outElectron.mvaIsoWPLoose = idBit(mvaIsoWPLooseName_);

      outElectron.mvaVal = mvaValuesName_.empty() ? 0. : patElectron.userFloat(mvaValuesName_);
    }

    outElectron.conversionVeto = !ConversionTools::hasMatchedConversion(inElectron, conversionsHandle, beamSpot.position());

    auto&& chargeInfo(inElectron.chargeInfo());
    outElectron.tripleCharge = chargeInfo.isGsfCtfConsistent && chargeInfo.isGsfCtfScPixConsistent && chargeInfo.isGsfScPixConsistent;

    // outElectron.mvaCategory = mvaCategoriesMap ? (*mvaCategoriesMap)[inRef] : -1;
    outElectron.charge = inElectron.charge();

    outElectron.sieie = inElectron.full5x5_sigmaIetaIeta();
    outElectron.sipip = inElectron.full5x5_sigmaIphiIphi();
    outElectron.r9 = inElectron.r9();
    outElectron.hOverE = inElectron.hadronicOverEm();
    outElectron.dPhiIn = inElectron.deltaPhiSuperClusterTrackAtVtx();
    outElectron.ecalE = inElectron.ecalEnergy();
    outElectron.trackP = inElectron.ecalEnergy() / inElectron.eSuperClusterOverP();
   
    auto& gsfTrack(*inElectron.gsfTrack());
    if (vertices.size() != 0) {
      auto& pv(vertices.at(0));
      auto pos(pv.position());
      outElectron.dxy = std::abs(gsfTrack.dxy(pos));
      outElectron.dz = std::abs(gsfTrack.dz(pos));
    }
    else {
      outElectron.dxy = std::abs(gsfTrack.dxy());
      outElectron.dz = std::abs(gsfTrack.dz());
    }

    outElectron.nMissingHits = gsfTrack.hitPattern().numberOfAllHits(reco::HitPattern::MISSING_INNER_HITS);

    double scEta(std::abs(sc.eta()));

    auto& pfIso(inElectron.pfIsolationVariables());
    outElectron.chIso = pfIso.sumChargedHadronPt;
    outElectron.nhIso = pfIso.sumNeutralHadronEt;
    outElectron.phIso = pfIso.sumPhotonEt;
    outElectron.puIso = pfIso.sumPUPt;
    outElectron.isoPUOffset = combIsoEA_.getEffectiveArea(scEta) * rho;

    if (isPAT) {
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

    outElectron.trackIso = inElectron.dr03TkSumPt();

    auto&& seedRef(sc.seed());
    if (seedRef.isNonnull()) {
      auto& seed(*seedRef);
      
      auto* seedHit(findHit(seed.seed()));
      if (seedHit)
        outElectron.eseed = seedHit->energy();

      outElectron.dEtaInSeed = inElectron.deltaEtaSuperClusterTrackAtVtx() - sc.eta() + seed.eta();
    }
    else
      outElectron.dEtaInSeed = std::numeric_limits<float>::max();

    for (auto& photon : photons) {
      if (photon.superCluster() == scRef) {
        auto* patPhoton(dynamic_cast<pat::Photon const*>(&photon));
        if (patPhoton != nullptr) {
          outElectron.chIsoPh = patPhoton->userFloat("phoChargedIsolation");
          outElectron.nhIsoPh = patPhoton->userFloat("phoNeutralHadronIsolation");
          outElectron.phIsoPh = patPhoton->userFloat("phoPhotonIsolation");
        }
      }
    }

    reco::CandidatePtr matchedPF(findPF(inElectron));

    if (matchedPF.isNonnull())
      outElectron.pfPt = matchedPF->pt();

    if (fillCorrectedPts_ && isPAT) {
      auto& patElectron(static_cast<pat::Electron const&>(inElectron));
      outElectron.smearedPt = patElectron.pt() * (1. + patElectron.userFloat("energySigmaValue"));
      outElectron.regPt = patElectron.pt() * patElectron.userFloat("ecalTrkEnergyPostCorr") / patElectron.energy();
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
  auto& pfMap(_objectMaps.at("pfCandidates").get<reco::Candidate, panda::PFCand>().fwdMap);
  auto& vtxMap(_objectMaps.at("vertices").get<reco::Vertex, panda::RecoVertex>().fwdMap);

  for (auto& link : scEleMap.bwdMap) { // panda -> edm
    auto& outElectron(*link.first);
    auto& scPtr(link.second);

    outElectron.superCluster.setRef(scMap.at(scPtr));
  }

  for (auto& link : pfEleMap.bwdMap) { // panda -> edm
    auto& outElectron(*link.first);
    auto& pfPtr(link.second);

    outElectron.matchedPF.setRef(pfMap.at(pfPtr));
  }

  for (auto& link : vtxEleMap.bwdMap) { // panda -> edm
    auto& outElectron(*link.first);
    auto& vtxPtr(link.second);

    outElectron.vertex.setRef(vtxMap.at(vtxPtr));
  }

  if (!isRealData_) {
    auto& genEleMap(objectMap_->get<reco::Candidate, panda::Electron>("gen"));

    auto& genMap(_objectMaps.at("genParticles").get<reco::Candidate, panda::GenParticle>().fwdMap);

    for (auto& link : genEleMap.bwdMap) {
      auto& genPtr(link.second);
      if (genMap.find(genPtr) == genMap.end())
        continue;

      auto& outElectron(*link.first);
      outElectron.matchedGen.setRef(genMap.at(genPtr));
    }
  }
}

DEFINE_TREEFILLER(ElectronsFiller);
