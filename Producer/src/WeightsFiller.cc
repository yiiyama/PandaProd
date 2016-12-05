#include "../interface/WeightsFiller.h"

#include "FWCore/Framework/interface/Run.h"

#include "PandaTree/Framework/interface/IOUtils.h"

#include "TDOMParser.h"
#include "TXMLNode.h"
#include "TXMLAttr.h"

WeightsFiller::WeightsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  if (!isRealData_) {
    getToken_(genInfoToken_, _cfg, _coll, "genEventInfo");
    getToken_(lheEventToken_, _cfg, _coll, "lheEvent");
    getTokenRun_(lheRunToken_, _cfg, _coll, "lheRun");
  }
}

void
WeightsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  // reweightDW booked dynamically at the first event
  _eventBranches.push_back("!reweightDW");
}

void
WeightsFiller::addOutput(TFile& _outputFile)
{
  outputFile_ = &_outputFile;
  _outputFile.cd();
   hSumW_ = new TH1D("hSumW", "SumW", 1, 0., 1.);
  hSumW_->GetXaxis()->SetBinLabel(1, "Nominal");
}

void
WeightsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  if (_inEvent.isRealData())
    _outEvent.weight = 1.;
  else {
    auto& genInfo(getProduct_(_inEvent, genInfoToken_));
    _outEvent.weight = genInfo.weight();

    auto* lheEvent(getProductSafe_(_inEvent, lheEventToken_));
    if (lheEvent) {
      if (weightIndices_.size() == 0) {
        // first event
        if (lheEvent->weights().size() > sizeof(_outEvent.reweightDW))
          throw std::runtime_error("Too many reweight factors in input");

        initIndices_(*lheEvent);
        auto* events(static_cast<TTree*>(outputFile_->Get("events")));
        panda::utils::book(*events, "", "reweightDW", TString::Format("[%d]", int(lheEvent->weights().size())), 'F', _outEvent.reweightDW, {"*"});
      }

      std::fill_n(_outEvent.reweightDW, sizeof(_outEvent.reweightDW), 0.);

      for (auto& wgt : lheEvent->weights()) {
        unsigned idx(weightIndices_.at(wgt.id));
        _outEvent.reweightDW[idx] = wgt.wgt / lheEvent->originalXWGTUP() - 1.;
      }
    }
  }
}

void
WeightsFiller::fillAll(edm::Event const& _inEvent, edm::EventSetup const&)
{
  if (_inEvent.isRealData())
    hSumW_->Fill(0.5);
  else {
    auto& genInfo(getProduct_(_inEvent, genInfoToken_));
    hSumW_->Fill(0.5, genInfo.weight());

    auto* lheEvent(getProductSafe_(_inEvent, lheEventToken_));
    if (lheEvent) {
      if (weightIndices_.size() == 0) {
        // first event
        initIndices_(*lheEvent);
      }

      for (auto& wgt : lheEvent->weights()) {
        unsigned idx(weightIndices_.at(wgt.id));
        hSumW_->Fill(idx + 1.5, wgt.wgt);
      }
    }
  }
}

void
WeightsFiller::fillEndRun(panda::Run& _outRun, edm::Run const& _inRun, edm::EventSetup const&)
{
  // Set weight names

  // LHERunInfoProduct (and GenRunInfoProduct FWIW) have mergeProduct method which forbids them
  // from being fetched in beginRun

  // Return if no reweight factors are set
  if (weightIndices_.size() == 0)
    return;

  auto* lheRun(getProductSafe_(_inRun, lheRunToken_));
  if (!lheRun)
    return;

  std::map<TString, unsigned> weightGroups;
  std::map<TString, TString> weightTitles;

  TTree* groupTree(0);
  TString* gcombine(new TString);
  TString* gtype(new TString);
  {
    TDirectory::TContext context(outputFile_);
    groupTree = new TTree("weightGroups", "weightGroups");
    groupTree->Branch("combine", "TString", &gcombine);
    groupTree->Branch("type", "TString", &gtype);
  }

  for (auto&& hItr(lheRun->headers_begin()); hItr != lheRun->headers_end(); ++hItr) {
    auto& lheHdr(*hItr);

    if (lheHdr.tag() != "initrwgt")
      continue;

    // xml document needs a root node
    TString buffer("<initrwgt>");

    // append lines stripping whitespaces and newlines
    for (auto&& line : lheHdr.lines()) {
      TString text(line);
      text = text.Strip(TString::kBoth, ' ');
      text = text.Strip(TString::kBoth, '\n');
      buffer += text;
    }

    // clean up the buffer - at least remove trailing open bracket (yes that happens sometimes)
    int len(buffer.Length());
    for (; len != 0; --len) {
      if (buffer[len - 1] == '>')
        break;
    }
    buffer = buffer(0, len);

    buffer += "</initrwgt>";

    TDOMParser parser;
    parser.SetValidate(false); // we don't define XML namespace etc.
    parser.ParseBuffer(buffer.Data(), buffer.Length());
    auto& document(*parser.GetXMLDocument());
    auto& rootnode(*document.GetRootNode());

    unsigned iG(0);

    // traverse through the XML document and collect all reweight factor names
    auto* wgNode(rootnode.GetChildren());
    do {
      // loop over weightgroup nodes

      auto* weightNode(wgNode->GetChildren());
      do {
        // loop over weight definition nodes

        if (std::strcmp(weightNode->GetNodeName(), "weight") != 0)
          continue;

        TList* attr(weightNode->GetAttributes());
        if (!attr)
          continue;

        auto* wid(static_cast<TXMLAttr*>(attr->FindObject("id")));
        if (!wid)
          continue;

        TString id(wid->GetValue());

        weightGroups[id] = iG;

        // weight definition is a content of the child (text) node
        auto* contentNode = weightNode->GetChildren();
        if (!contentNode || contentNode->GetNodeType() != TXMLNode::kXMLTextNode)
          continue;

        TString content(contentNode->GetContent());
        content.Strip(TString::kBoth);

        weightTitles[id] = content;
      }
      while ((weightNode = weightNode->GetNextNode()));

      // weightgroup attributes
      TList& attributes(*wgNode->GetAttributes());
          
      auto* combine(static_cast<TXMLAttr*>(attributes.FindObject("combine")));
      if (combine)
        *gcombine = combine->GetValue();
      else
        *gcombine = "";

      auto* type(static_cast<TXMLAttr*>(attributes.FindObject("type")));
      if (type)
        *gtype = type->GetValue();
      else
        *gtype = "";

      groupTree->Fill();
    }
    while ((wgNode = wgNode->GetNextNode()));

    // we don't need to look at any other LHE headers
    break;
  }

  TDirectory::TContext context(outputFile_);

  groupTree->Write();
  delete groupTree;
  delete gcombine;
  delete gtype;

  std::vector<TString> orderedIds(weightIndices_.size());
  for (auto& ind : weightIndices_)
    orderedIds[ind.second] = ind.first;

  auto* weightTree(new TTree("weights", "weights"));
  auto* wid(new TString);
  auto* wtitle(new TString);
  unsigned short gid(0);
  weightTree->Branch("id", "TString", &wid);
  weightTree->Branch("title", "TString", &wtitle);
  weightTree->Branch("group", &gid, "group/s");

  for (TString& id : orderedIds) {
    *wid = id;
    *wtitle = weightTitles.at(id);
    gid = weightGroups.at(id);
    weightTree->Fill();
  }

  weightTree->Write();
  delete weightTree;
  delete wid;
  delete wtitle;
}

void
WeightsFiller::initIndices_(LHEEventProduct const& _lheEvent)
{
 
  hSumW_->SetBins(_lheEvent.weights().size() + 1, 0., _lheEvent.weights().size() + 1.);

  for (auto& wgt : _lheEvent.weights()) {
    unsigned idx(weightIndices_.size());
    weightIndices_[wgt.id] = idx;
  }
}

DEFINE_TREEFILLER(WeightsFiller);
