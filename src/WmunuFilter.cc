//
// Original Author:  thomas.mccauley@cern.ch
//

#include <memory>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"

#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/METReco/interface/PFMETCollection.h"

#include <iostream>
#include <string>
#include <fstream>

class WmunuFilter : public edm::EDFilter 
{
public:
  explicit WmunuFilter(const edm::ParameterSet&);
  ~WmunuFilter();

private:
  virtual void beginJob() ;
  virtual bool filter(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  edm::InputTag muonInputTag_;
  edm::InputTag pfMETInputTag_;

  std::ofstream csvOut_;
  std::string csvFileName_;
  double minMuonPt_;
  double maxMuonEta_;
};

WmunuFilter::WmunuFilter(const edm::ParameterSet& iConfig)
  : muonInputTag_(iConfig.getParameter<edm::InputTag>("muonInputTag")),
    pfMETInputTag_(iConfig.getParameter<edm::InputTag>("pfMETInputTag")),
    csvFileName_(iConfig.getParameter<std::string>("csvFileName")),
    minMuonPt_(iConfig.getParameter<double>("minMuonPt")),
    maxMuonEta_(iConfig.getParameter<double>("maxMuonEta"))
{    
  csvOut_.open(csvFileName_.c_str());
}

WmunuFilter::~WmunuFilter()
{}

bool
WmunuFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<reco::MuonCollection> muons;
  iEvent.getByLabel(muonInputTag_, muons);

  if ( ! muons.isValid() ) 
  {    
    std::cerr<<"WmunuFilter: invalid muon collection"<<std::endl;
    return false;
  }

  double MET, phiMET;

  edm::Handle<reco::PFMETCollection> pfMets;
  iEvent.getByLabel(pfMETInputTag_, pfMets);

  if ( ! pfMets.isValid() ) 
  {  
    std::cerr<<"WmunuFilter: invalid PFMET collection"<<std::endl;
    return false;
  }

  unsigned int nMET = 0;

  for ( reco::PFMETCollection::const_iterator it = pfMets->begin(), itEnd = pfMets->end(); it != itEnd; ++it )
  { 
    nMET++;
    phiMET = (*it).phi();
    MET = (*it).pt();  
  }

  if ( nMET != 1 )
    return false;
  

  double pt, eta, iso, chiSq, dxy;
  unsigned int nMuons = 0;

  double muPt, muEta, muPhi, Q;

  for ( reco::MuonCollection::const_iterator it = muons->begin(), end = muons->end();
        it != end; ++it ) 
  {
    // We want one (and only one) global muon that satisfies the requirements

    if ( (*it).isGlobalMuon() ) 
    {   
      pt = (*it).globalTrack()->pt();      
      eta = (*it).globalTrack()->eta();
      iso = (*it).isolationR03().sumPt + (*it).isolationR03().emEt + (*it).isolationR03().hadEt;
      chiSq = (*it).globalTrack()->normalizedChi2();
      dxy = (*it).globalTrack()->dxy();

      if ( pt > 10 && fabs(eta) < 2.4 && nMuons >= 1 )
        return false;

      if ( pt > minMuonPt_ && fabs(eta) < maxMuonEta_ )
      {
        nMuons++;

        muPt = pt;
        muEta = eta;
        muPhi = (*it).globalTrack()->phi();
        Q = (*it).globalTrack()->charge();
      }   
    }
  }
  
  if ( nMuons == 1 ) 
  {
    csvOut_<< iEvent.id().run()<<","<< iEvent.id().event() <<","<< muPt <<","<< muEta <<","<< muPhi <<","<< Q <<","<< chiSq <<","<< dxy <<","<< iso <<","<< MET <<","<< phiMET <<std::endl;
    return true;
  }
  
  return false;
}

void 
WmunuFilter::beginJob()
{
  csvOut_<<"Run,Event,pt,eta,phi,Q,chiSq,dxy,iso,MET,phiMET"<<std::endl;
}

void 
WmunuFilter::endJob() {
}

DEFINE_FWK_MODULE(WmunuFilter);
