// -*- C++ -*-
// 
/**\class PuppiCandidatesProducer

   Description: Compute the maximum charged hadron isolation value over all PV hypotheses.

   Implementation:
   Create PF candidates with puppi weights from the input packed candidates.
*/
//
// Original Author:  Yutaro Iiyama
//         Created:  Mon, 26 Jun 2017 21:55:00 GMT
//
//

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"

#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"

#include "PandaProd/Auxiliary/interface/getProduct.h"

#include <memory>
#include <vector>

class PuppiCandidatesProducer : public edm::stream::EDProducer<> {
public:
  explicit PuppiCandidatesProducer(const edm::ParameterSet&);
  ~PuppiCandidatesProducer();
  
private:
  void produce(edm::Event&, edm::EventSetup const&) override;

  edm::EDGetTokenT<pat::PackedCandidateCollection> packedCandidatesToken_;
};

PuppiCandidatesProducer::PuppiCandidatesProducer(edm::ParameterSet const& _cfg) :
  packedCandidatesToken_(consumes<pat::PackedCandidateCollection>(_cfg.getParameter<edm::InputTag>("src")))
{
  produces<reco::PFCandidateCollection>();
}

PuppiCandidatesProducer::~PuppiCandidatesProducer()
{
}

void
PuppiCandidatesProducer::produce(edm::Event& _event, edm::EventSetup const&)
{
  // Product
  std::unique_ptr<reco::PFCandidateCollection> output(new reco::PFCandidateCollection);

  // Inputs
  auto& srcCandidates(*getProduct(_event, packedCandidatesToken_));

  static reco::PFCandidate const idTranslator;

  for (auto& cand : srcCandidates) {
    auto p4(cand.p4());
    p4 *= cand.puppiWeight();
    output->emplace_back(cand.charge(), p4, idTranslator.translatePdgIdToType(cand.pdgId()));
  }

  _event.put(std::move(output));
}

DEFINE_FWK_MODULE(PuppiCandidatesProducer);
