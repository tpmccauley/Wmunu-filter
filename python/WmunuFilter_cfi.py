import FWCore.ParameterSet.Config as cms

WmunuFilter = cms.EDFilter('WmunuFilter',
                           muonInputTag = cms.InputTag("muons"),
                           pfMETInputTag = cms.InputTag("pfMet"),
                           csvFileName = cms.string("Wmunu.csv"),
                           minMuonPt = cms.double(30.0),
                           maxMuonEta = cms.double(2.1),
                           maxRelIso = cms.double(0.15)
                           )
