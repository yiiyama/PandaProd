#include "../interface/FatJetsFiller.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/Math/interface/deltaR.h"

FatJetsFiller::FatJetsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  JetsFiller(_name, _cfg, _coll),
  njettinessTag_(getParameter_<std::string>(_cfg, "njettiness")),
  sdKinematicsTag_(getParameter_<std::string>(_cfg, "sdKinematics")),
  activeArea_(7., 1, 0.01),
  areaDef_(fastjet::active_area_explicit_ghosts, activeArea_),
  computeSubstructure_(getParameter_<bool>(_cfg, "computeSubstructure", false))
{
  getToken_(subjetsToken_, _cfg, _coll, "subjets", false);
  getToken_(btagsToken_, _cfg, _coll, "btags", false);
  getToken_(qglToken_, _cfg, _coll, "qgl", false);

  if (computeSubstructure_) {
    jetDefCA_ = new fastjet::JetDefinition(fastjet::cambridge_algorithm, R_);
    softdrop_ = new fastjet::contrib::SoftDrop(1., 0.15, R_);
    ecfnManager_ = new ECFNManager();
    tau_ = new fastjet::contrib::Njettiness(fastjet::contrib::OnePass_KT_Axes(), fastjet::contrib::NormalizedMeasure(1., R_));

    //htt
    bool optimalR=true; bool doHTTQ=false;
    double minSJPt=0.; double minCandPt=0.;
    double sjmass=30.; double mucut=0.8;
    double filtR=0.3; int filtN=5;
    int mode=4; double minCandMass=0.;
    double maxCandMass=9999999.; double massRatioWidth=9999999.;
    double minM23Cut=0.; double minM13Cut=0.;
    double maxM13Cut=9999999.;  bool rejectMinR=false;
    htt_ = new fastjet::HEPTopTaggerV2(optimalR,doHTTQ,
                                        minSJPt,minCandPt,
                                        sjmass,mucut,
                                        filtR,filtN,
                                        mode,minCandMass,
                                        maxCandMass,massRatioWidth,
                                        minM23Cut,minM13Cut,
                                        maxM13Cut,rejectMinR);
  }
}

FatJetsFiller::~FatJetsFiller()
{
  delete jetDefCA_;
  delete softdrop_;
  delete ecfnManager_;
  delete tau_;
  delete htt_;
}

void
FatJetsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList& _runBranches) const
{
  JetsFiller::branchNames(_eventBranches, _runBranches);
  _eventBranches.emplace_back("!" + getName() + ".matchedGenJet_");

  if (!computeSubstructure_) {
    char const* substrBranches[] = {
      ".tau1SD",
      ".tau2SD",
      ".tau3SD",
      ".htt_mass",
      ".htt_frec",
      ".ecfs"
    };
    for (char const* b : substrBranches)
      _eventBranches.emplace_back("!" + getName() + b);
  }
}

void
FatJetsFiller::fillDetails_(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inSubjets(getProduct_(_inEvent, subjetsToken_));

  reco::JetTagCollection const* inBtags(0);
  if (!btagsToken_.second.isUninitialized())
    inBtags = &getProduct_(_inEvent, btagsToken_);

  FloatMap const* inQGL(0);
  if (!qglToken_.second.isUninitialized())
    inQGL = &getProduct_(_inEvent, qglToken_);

  double betas[] = {0.5, 1., 2., 4.};

  std::string substrLabel;
  if (getName() == "chsAK8Jets")
    substrLabel = "AK8PFchs";
  else if (getName() == "chsCA15Jets")
    substrLabel = "CA15PFchs";
  else if (getName() == "puppiAK8Jets")
    substrLabel = "AK8PFPuppi";
  else if (getName() == "puppiCA15Jets")
    substrLabel = "CA15PFPuppi";

  auto& outSubjets(_outEvent.subjets);

  typedef std::vector<fastjet::PseudoJet> VPseudoJet;

  auto& jetMap(objectMap_->get<reco::Jet, panda::PJet>());

  for (auto& link : jetMap.bwdMap) { // panda -> edm
    auto& outJet(static_cast<panda::PFatJet&>(*link.first));

    if (dynamic_cast<pat::Jet const*>(link.second.get())) {
      auto& inJet(static_cast<pat::Jet const&>(*link.second));

      outJet.tau1 = inJet.userFloat(njettinessTag_ + ":tau1");
      outJet.tau2 = inJet.userFloat(njettinessTag_ + ":tau2");
      outJet.tau3 = inJet.userFloat(njettinessTag_ + ":tau3");
      outJet.mSD  = inJet.userFloat(sdKinematicsTag_ + ":Mass");

      unsigned iSJ(-1);
      for (auto& inSubjet : inSubjets) {
        ++iSJ;
        if (reco::deltaR(inSubjet.eta(), inSubjet.phi(), inJet.eta(), inJet.phi()) > R_) 
          continue;

        auto& outSubjet(outSubjets.create_back());

        fillP4(outSubjet, inSubjet);

        auto&& inRef(inSubjets.refAt(iSJ));

        if (inBtags)
          outSubjet.csv = (*inBtags)[inRef];
        if (inQGL)
          outSubjet.qgl = (*inQGL)[inRef];

        outJet.subjets.push_back(outSubjet);
      }

      // reset the ECFs
      for (unsigned iB(0); iB != 4; ++iB) {
        for (int N : {1, 2, 3, 4}) {
          for (int order : {1, 2, 3}) {
            outJet.set_ecf(order, N, iB, -1);
          }
        }
      }

      if (computeSubstructure_) {
        // either we want to associate to pf cands OR compute extra info about the first or second jet
        // but do not do any of this if ReduceEvent() is tripped

        // calculate ECFs, groomed tauN
        VPseudoJet vjet;
        for (auto&& ptr : inJet.getJetConstituents()) { 
          // create vector of PseudoJets
          auto& cand(*ptr);
          if (cand.pt() < 0.01) 
            continue;

          vjet.emplace_back(cand.px(), cand.py(), cand.pz(), cand.energy());
        }

        fastjet::ClusterSequenceArea seq(vjet, *jetDefCA_, areaDef_);
        VPseudoJet alljets(fastjet::sorted_by_pt(seq.inclusive_jets(0.1)));

        if (alljets.size() > 0){
          fastjet::PseudoJet& leadingJet(alljets[0]);
          fastjet::PseudoJet sdJet((*softdrop_)(leadingJet));

          // get and filter constituents of groomed jet
          VPseudoJet sdconsts(fastjet::sorted_by_pt(sdJet.constituents()));
          unsigned nFilter(std::min(100, int(sdconsts.size())));
          VPseudoJet sdconstsFiltered(sdconsts.begin(), sdconsts.begin() + nFilter);

          // calculate ECFs
          for (unsigned iB(0); iB != 4; ++iB) {
            calcECFN(betas[iB], sdconstsFiltered, ecfnManager_); // calculate for all Ns and os
            for (int N : {1, 2, 3, 4}) {
              for (int order : {1, 2, 3}) {
                float ecf(ecfnManager_->ecfns[TString::Format("%i_%i", N, order)]);
                if (!outJet.set_ecf(order, N, iB, ecf))
                  throw std::runtime_error(TString::Format("FatJetsFiller Could not save o=%i, N=%i, iB=%i", order, N, iB).Data());
              } // o loop
            } // N loop
          } // beta loop

          outJet.tau3SD = tau_->getTau(3, sdconsts);
          outJet.tau2SD = tau_->getTau(2, sdconsts);
          outJet.tau1SD = tau_->getTau(1, sdconsts);

          // HTT
          fastjet::PseudoJet httJet(htt_->result(leadingJet));
          if (httJet != 0) {
            auto* s(static_cast<fastjet::HEPTopTaggerV2Structure*>(httJet.structure_non_const_ptr()));
            outJet.htt_mass = s->top_mass();
            outJet.htt_frec = s->fRec();
          }
        }
        else
          throw std::runtime_error("PandaProd::Ntupler::FatJetFiller: Jet could not be clustered");

      } // if computeSubstructure_
    }
  }
}

void
FatJetsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  if (fillConstituents_) {
    auto& jetMap(objectMap_->get<reco::Jet, panda::PJet>());

    auto& pfMap(_objectMaps.at("pfCandidates").get<reco::Candidate, panda::PPFCand>().fwdMap);

    for (auto& link : jetMap.fwdMap) { // edm -> panda
      auto& inJet(*link.first);
      auto& outJet(static_cast<panda::PFatJet&>(*link.second));

      for (auto&& ptr : inJet.getJetConstituents()) {
        reco::CandidatePtr p(ptr);
        while (p->sourceCandidatePtr(0).isNonnull())
          p = p->sourceCandidatePtr(0);
        outJet.constituents.push_back(*pfMap.at(p));
      }
    }
  }
}

DEFINE_TREEFILLER(FatJetsFiller);
