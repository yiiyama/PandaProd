#ifndef PandaProd_Producer_RhoFiller_h
#define PandaProd_Producer_RhoFiller_h

#include "FillerBase.h"

class RhoFiller : public FillerBase {
 public:
  RhoFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~RhoFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  edm::EDGetTokenT<double> rhoToken_;
};

#endif
