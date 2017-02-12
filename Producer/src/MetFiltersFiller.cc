#include "../interface/MetFiltersFiller.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/PatCandidates/interface/MET.h"

MetFiltersFiller::MetFiltersFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  for (auto& proc : getParameter_<VString>(_cfg, "filterProcesses"))
    filterResultsTokens_.emplace_back("TriggerResults:" + proc, _coll.consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", proc)));

  if (!getParameter_<std::string>(_cfg, "dupECALClusters", "").empty())
    getToken_(dupECALClustersToken_, _cfg, _coll, "dupECALClusters");
  if (!getParameter_<std::string>(_cfg, "unfixedECALHits", "").empty())
    getToken_(unfixedECALHitsToken_, _cfg, _coll, "unfixedECALHits");
}

void
MetFiltersFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("metFilters");
  if (dupECALClustersToken_.second.isUninitialized())
    _eventBranches.emplace_back("!metFilters.dupECALClusters");
  if (unfixedECALHitsToken_.second.isUninitialized())
    _eventBranches.emplace_back("!metFilters.unfixedECALHits");
}

void
MetFiltersFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& outMetFilters(_outEvent.metFilters);

  for (auto& token : filterResultsTokens_) {
    auto& inFilterResults(getProduct_(_inEvent, token));

    auto&& filterNames(_inEvent.triggerNames(inFilterResults));
    for (unsigned iF(0); iF != filterNames.size(); ++iF) {
      auto& name(filterNames.triggerName(iF));
    
      if (name == "Flag_HBHENoiseFilter")
        outMetFilters.hbhe = !inFilterResults.accept(iF);
      else if (name == "Flag_HBHENoiseIsoFilter")
        outMetFilters.hbheIso = !inFilterResults.accept(iF);
      else if (name == "Flag_EcalDeadCellTriggerPrimitiveFilter")
        outMetFilters.ecalDeadCell = !inFilterResults.accept(iF);
      else if (name == "Flag_eeBadScFilter")
        outMetFilters.badsc = !inFilterResults.accept(iF);
      else if (name == "Flag_globalTightHalo2016Filter")
        outMetFilters.globalHalo16 = !inFilterResults.accept(iF);
      else if (name == "Flag_badMuons") // reverse convention
        outMetFilters.badMuons = inFilterResults.accept(iF);
      else if (name == "Flag_duplicateMuons") // reverse convention
        outMetFilters.duplicateMuons = inFilterResults.accept(iF);
    }
  }

  if (!dupECALClustersToken_.second.isUninitialized())
    outMetFilters.dupECALClusters = getProduct_(_inEvent, dupECALClustersToken_);

  if (!unfixedECALHitsToken_.second.isUninitialized())
    outMetFilters.unfixedECALHits = (getProduct_(_inEvent, unfixedECALHitsToken_).size() != 0);
}

DEFINE_TREEFILLER(MetFiltersFiller);
