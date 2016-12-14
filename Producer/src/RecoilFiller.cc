#include "../interface/RecoilFiller.h"

RecoilFiller::RecoilFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(categoriesToken_, _cfg, _coll, "categories");
  getToken_(maxToken_, _cfg, _coll, "max");
}

void
RecoilFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  _outEvent.recoil.categories = getProduct_(_inEvent, categoriesToken_);
  _outEvent.recoil.max = getProduct_(_inEvent, maxToken_);
}

DEFINE_TREEFILLER(RecoilFiller);
