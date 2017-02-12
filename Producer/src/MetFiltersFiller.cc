#include "../interface/MetFiltersFiller.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/PatCandidates/interface/MET.h"

MetFiltersFiller::MetFiltersFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(filterResultsToken_, _cfg, _coll, "generalFilters");
}

void
MetFiltersFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("metFilters");
}

void
MetFiltersFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inFilterResults(getProduct_(_inEvent, filterResultsToken_));

  auto& outMetFilters(_outEvent.metFilters);

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
  }
}

DEFINE_TREEFILLER(MetFiltersFiller);
