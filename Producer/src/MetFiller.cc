#include "../interface/MetFiller.h"

#include "DataFormats/PatCandidates/interface/MET.h"

MetFiller::MetFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  fillOthers_(getParameter_<bool>(_cfg, "fillOthers", false))
{
  if (_name == "t1Met")
    outputType_ = kPF;
  else if (_name == "puppiMet")
    outputType_ = kPuppi;

  getToken_(metToken_, _cfg, _coll, "met");
  if (fillOthers_) {
    getToken_(noHFMetToken_, _cfg, _coll, "noHFMet");
    getToken_(candidatesToken_, _cfg, _coll, "common", "pfCandidates");
  }
}

void
MetFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  if (isRealData_) {
    if (fillOthers_)
      _eventBranches.emplace_back("!genMet");

    char const* skipped[] = {
      ".ptSmear",
      ".ptSmearUp",
      ".ptSmearDown",
      ".phiSmear",
      ".phiSmearUp",
      ".phiSmearDown"
    };

    for (char const* b : skipped)
      _eventBranches.emplace_back("!" + getName() + b);
  }
}

void
MetFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inMet(getProduct_(_inEvent, metToken_).at(0));

  panda::RecoMet* pOutMet(0);
  switch (outputType_) {
  case kPF:
    pOutMet = &_outEvent.met;
    break;
  case kPuppi:
    pOutMet = &_outEvent.puppiMet;
    break;
  }
  panda::RecoMet& outMet(*pOutMet);

  outMet.pt = inMet.pt();
  outMet.phi = inMet.phi();

  auto* patMet(dynamic_cast<pat::MET const*>(&inMet));

  if (patMet) {
    outMet.sumETRaw = patMet->uncorSumEt();

    outMet.ptCorrUp = patMet->shiftedPt(pat::MET::JetEnUp);
    outMet.phiCorrUp = patMet->shiftedPhi(pat::MET::JetEnUp);
    outMet.ptCorrDown = patMet->shiftedPt(pat::MET::JetEnDown);
    outMet.phiCorrDown = patMet->shiftedPhi(pat::MET::JetEnDown);

    outMet.ptUnclUp = patMet->shiftedPt(pat::MET::UnclusteredEnUp);
    outMet.phiUnclUp = patMet->shiftedPhi(pat::MET::UnclusteredEnUp);
    outMet.ptUnclDown = patMet->shiftedPt(pat::MET::UnclusteredEnDown);
    outMet.phiUnclDown = patMet->shiftedPhi(pat::MET::UnclusteredEnDown);
  }

  if (fillOthers_) {
    auto& inNoHFMet(getProduct_(_inEvent, noHFMetToken_).at(0));
    auto& candidates(getProduct_(_inEvent, candidatesToken_));

    if (patMet) {
      if (!isRealData_) {
        _outEvent.genMet.pt = patMet->genMET()->pt();
        _outEvent.genMet.phi = patMet->genMET()->phi();
      }

      _outEvent.rawMet.pt = patMet->uncorPt();
      _outEvent.rawMet.phi = patMet->uncorPhi();

      _outEvent.caloMet.pt = patMet->caloMETPt();
      _outEvent.caloMet.phi = patMet->caloMETPhi();
    }

    _outEvent.noHFMet.pt = inNoHFMet.pt();
    _outEvent.noHFMet.phi = inNoHFMet.phi();

    double noMuMex(inMet.px());
    double noMuMey(inMet.py());
    double trkMex(0.);
    double trkMey(0.);
    double neutralMex(0.);
    double neutralMey(0.);
    double photonMex(0.);
    double photonMey(0.);
    double hfMex(0.);
    double hfMey(0.);
    for (auto& cand : candidates) {
      if (std::abs(cand.pdgId()) == 13) {
        noMuMex += cand.px();
        noMuMey += cand.py();
      }
      else if (cand.pdgId() == 130) {
        neutralMex -= cand.px();
        neutralMey -= cand.py();
      }
      else if (cand.pdgId() == 22) {
        photonMex -= cand.px();
        photonMey -= cand.py();
      }
      else if (cand.pdgId() == 1 || cand.pdgId() == 2) {
        hfMex -= cand.px();
        hfMey -= cand.py();
      }

      if (cand.charge() != 0) {
        trkMex -= cand.px();
        trkMey -= cand.py();
      }
    }

    _outEvent.noMuMet.setXY(noMuMex, noMuMey);
    _outEvent.trkMet.setXY(trkMex, trkMey);
    _outEvent.neutralMet.setXY(neutralMex, neutralMey);
    _outEvent.photonMet.setXY(photonMex, photonMey);
    _outEvent.hfMet.setXY(hfMex, hfMey);
  }
}

DEFINE_TREEFILLER(MetFiller);
