#ifndef PandaProd_Producer_WeightsFiller_h
#define PandaProd_Producer_WeightsFiller_h

#include "FillerBase.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

class WeightsFiller : public FillerBase {
 public:
  WeightsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~WeightsFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  edm::EDGetTokenT<GenEventInfoProduct> genInfoToken_;
};

#endif
