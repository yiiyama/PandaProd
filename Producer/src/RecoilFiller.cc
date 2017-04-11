#include "../interface/RecoilFiller.h"

RecoilFiller::RecoilFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(categoriesToken_, _cfg, _coll, "categories");
  getToken_(maxToken_, _cfg, _coll, "max");
}

void
RecoilFiller::addOutput(TFile& _outputFile)
{
  TDirectory::TContext context(&_outputFile);
  auto* t(panda::utils::makeDocTree("RecoilCategory", panda::Recoil::CategoryName, panda::Recoil::nCategories));
  t->Write();
  delete t;
}

void
RecoilFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("recoil");
}

void
RecoilFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  int categories(getProduct_(_inEvent, categoriesToken_));
  auto& recoil(_outEvent.recoil);

  recoil.met = ((categories >> panda::Recoil::rMET) & 1) != 0;
  recoil.monoMu = ((categories >> panda::Recoil::rMonoMu) & 1) != 0;
  recoil.monoE = ((categories >> panda::Recoil::rMonoE) & 1) != 0;
  recoil.diMu = ((categories >> panda::Recoil::rDiMu) & 1) != 0;
  recoil.diE = ((categories >> panda::Recoil::rDiE) & 1) != 0;
  recoil.gamma = ((categories >> panda::Recoil::rGamma) & 1) != 0;

  recoil.max = getProduct_(_inEvent, maxToken_);
}

DEFINE_TREEFILLER(RecoilFiller);
