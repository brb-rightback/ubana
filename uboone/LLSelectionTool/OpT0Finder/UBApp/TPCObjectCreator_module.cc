////////////////////////////////////////////////////////////////////////
// Class:       TPCObjectCreator
// Module Type: producer
// File:        TPCObjectCreator_module.cc
//
// Generated at Mon Oct 31 07:46:33 2016 by Marco Del Tutto using artmod
// from cetpkgsupport v1_11_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"

#include <memory>

#include "nusimdata/SimulationBase/MCTruth.h"
#include "larsim/MCCheater/BackTracker.h"

#include "larpandora/LArPandoraInterface/LArPandoraHelper.h"
#include "lardataobj/RecoBase/PFParticle.h"
#include "lardataobj/RecoBase/Vertex.h"
#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/RecoBase/Cluster.h"
#include "lardataobj/RecoBase/Hit.h"

#include "TVector3.h"
#include "TTree.h"



class TPCObjectCreator;

class TPCObjectCreator : public art::EDProducer {
public:
  explicit TPCObjectCreator(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  TPCObjectCreator(TPCObjectCreator const &) = delete;
  TPCObjectCreator(TPCObjectCreator &&) = delete;
  TPCObjectCreator & operator = (TPCObjectCreator const &) = delete;
  TPCObjectCreator & operator = (TPCObjectCreator &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;


private:
  bool InFV(double, double, double);

  std::string  m_hitfinderLabel;         ///<
  std::string  m_spacepointLabel;        ///<
  std::string  m_particleLabel;          ///<
  std::string  m_geantModuleLabel;       ///<
  std::string  m_geniemodulelabel;       ///<

  int nPFPhierarchy_perEvent;                     ///< Number of PFP hierarchies per event

  TTree* _tree1;
  int _run, _subrun, _event;
  int _nPFP_nu, _nPFP_bkg;

  TTree* _tree2;
  int _nPFPhierarchy;
  int _recoNuVtxExists;
  int _trueVtxInFV;
  double _nu_vtxx;
  double _nu_vtxy;
  double _nu_vtxz;
  double _nu_e;
  double _lep_mom, _lep_dcosx, _lep_dcosy, _lep_dcosz;
  int _g4muonsForThisEvent;
  double _g4muon_mom, _g4muon_trklen;
  int _ccnc;
  int _nupdg;
  // Declare member data here.

};


TPCObjectCreator::TPCObjectCreator(fhicl::ParameterSet const & p)
// :
// Initialize member data here.
{

  m_particleLabel    = p.get<std::string>("PFParticleModule", "pandoraNu");
  m_hitfinderLabel   = p.get<std::string>("HitFinderModule",  "gaushit");
  m_geantModuleLabel = p.get<std::string>("GeantModule",      "largeant");
  m_spacepointLabel  = p.get<std::string>("SpacePointModule", "pandoraNu");
  m_geniemodulelabel = p.get<std::string>("GenieModuleLabel", "generator");



  art::ServiceHandle<art::TFileService> fs;

  _tree1 = fs->make<TTree>("tree_perPFPhierarchy","");
  _tree1->Branch("run",&_run,"run/I");
  _tree1->Branch("subrun",&_subrun,"subrun/I");
  _tree1->Branch("event",&_event,"event/I");
  _tree1->Branch("nPFP_nu", &_nPFP_nu, "nPFP_nu/I");
  _tree1->Branch("nPFP_bkg",&_nPFP_bkg,"nPFP_bkg/I");

  _tree2 = fs->make<TTree>("tree_perEvent","");
  _tree2->Branch("run",&_run,"run/I");
  _tree2->Branch("subrun",&_subrun,"subrun/I");
  _tree2->Branch("event",&_event,"event/I"); 
  _tree2->Branch("nPFPhierarchy",&_nPFPhierarchy,"nPFPhierarchy/I");
  _tree2->Branch("recoNuVtxExists", &_recoNuVtxExists, "recoNuVtxExists/I");
  _tree2->Branch("trueVtxInFV",&_trueVtxInFV,"trueVtxInFV/I");
  _tree2->Branch("nu_e",&_nu_e,"nu_e/D");
  _tree2->Branch("nu_vtxx",&_nu_vtxx,"nu_vtxx/D");
  _tree2->Branch("nu_vtxy",&_nu_vtxy,"nu_vtxy/D");
  _tree2->Branch("nu_vtxz",&_nu_vtxz,"nu_vtxz/D");
  _tree2->Branch("lep_mom",&_lep_mom,"lep_mom/D");
  _tree2->Branch("lep_dcosx",&_lep_dcosx,"lep_dcosx/D");
  _tree2->Branch("lep_dcosy",&_lep_dcosy,"lep_dcosy/D");
  _tree2->Branch("lep_dcosz",&_lep_dcosz,"lep_dcosz/D");
  _tree2->Branch("g4muonsForThisEvent",&_g4muonsForThisEvent,"g4muonsForThisEvent/I");
  _tree2->Branch("g4muon_mom",&_g4muon_mom,"g4muon_mom/D");
  _tree2->Branch("g4muon_trklen",&_g4muon_trklen,"g4muon_trklen/D");
  _tree2->Branch("ccnc",&_ccnc,"ccnc/I");
  _tree2->Branch("nupdg",&_nupdg,"nupdg/I");


 // Call appropriate produces<>() functions here.
}

void TPCObjectCreator::produce(art::Event & e)
{

  //Vectors and maps we will use to store Pandora information
  lar_pandora::PFParticleVector pfParticleList; //vector of PFParticles
  lar_pandora::PFParticlesToClusters pfParticleToClusterMap; //PFParticle-to-cluster map

  //Use LArPandoraHelper functions to collect Pandora information
  lar_pandora::LArPandoraHelper::CollectPFParticles(e, m_particleLabel, pfParticleList, pfParticleToClusterMap); //collect PFParticles and build map PFParticles to Clusters

  //
  lar_pandora::VertexVector           allPfParticleVertices;
  lar_pandora::PFParticlesToVertices  pfParticleToVertexMap;
  lar_pandora::LArPandoraHelper::CollectVertices(e, m_particleLabel, allPfParticleVertices, pfParticleToVertexMap);  
  lar_pandora::TrackVector            allPfParticleTracks;
  lar_pandora::PFParticlesToTracks    pfParticleToTrackMap;
  lar_pandora::LArPandoraHelper::CollectTracks(e, m_particleLabel, allPfParticleTracks, pfParticleToTrackMap);


  // Collecting GENIE particles
  art::Handle< std::vector<simb::MCTruth> > mctruthListHandle;	
  std::vector<art::Ptr<simb::MCTruth> > mclist;
  if (e.getByLabel(m_geniemodulelabel,mctruthListHandle))
    art::fill_ptr_vector(mclist, mctruthListHandle);

  // 
  art::ServiceHandle<cheat::BackTracker> bt;
  const sim::ParticleList& g4plist = bt->ParticleList();

  for (unsigned int n = 0; n < pfParticleList.size(); ++n) {

    const art::Ptr<recob::PFParticle> particle = pfParticleList.at(n);
    std::cout <<"PFParticle: " << particle->Self()
              << " IsPrimary? " << particle->IsPrimary()
              << " IsNeutrino? " << lar_pandora::LArPandoraHelper::IsNeutrino(particle)
              << " IsTrack? " << lar_pandora::LArPandoraHelper::IsTrack(particle)
              << " IsShower? " << lar_pandora::LArPandoraHelper::IsShower(particle)
    << std::endl;

  }

  std::cout << "*****************************************" << std::endl;

  nPFPhierarchy_perEvent = 0;

  // Get true nu vertex (assume 1 nu interaction per spill)
  int iList = 0;
  TVector3 mcNuVertex   (mclist[iList]->GetNeutrino().Nu().Vx(),mclist[iList]->GetNeutrino().Nu().Vy(),mclist[iList]->GetNeutrino().Nu().Vz());
  _nu_vtxx = mclist[iList]->GetNeutrino().Nu().Vx();
  _nu_vtxy = mclist[iList]->GetNeutrino().Nu().Vy();
  _nu_vtxz = mclist[iList]->GetNeutrino().Nu().Vz();
  _nu_e    = mclist[iList]->GetNeutrino().Nu().E();
  _ccnc    = mclist[iList]->GetNeutrino().CCNC();
  _nupdg   = mclist[iList]->GetNeutrino().Nu().PdgCode();

  // Also save muon (lepton) momentum
  _lep_mom =  mclist[iList]->GetNeutrino().Lepton().P();
  if (mclist[iList]->GetNeutrino().Lepton().P()){
    _lep_dcosx = mclist[iList]->GetNeutrino().Lepton().Px()/mclist[iList]->GetNeutrino().Lepton().P();
    _lep_dcosy = mclist[iList]->GetNeutrino().Lepton().Py()/mclist[iList]->GetNeutrino().Lepton().P();
    _lep_dcosz = mclist[iList]->GetNeutrino().Lepton().Pz()/mclist[iList]->GetNeutrino().Lepton().P();
  }

  // Also save geant lepton track
  sim::ParticleList::const_iterator itPart = g4plist.begin(),
                                    pend   = g4plist.end(); // iterator to pairs (track id, particle)

  std::string pri("primary");
  int isGENIE = 1;
  int isMuonPlus = 13;
  _g4muonsForThisEvent = 0;

  // Loop over GEANT particles
  for(size_t iPart = 0; (iPart < g4plist.size()) && (itPart != pend); ++iPart){
    const simb::MCParticle* pPart = (itPart++)->second;
    if (!pPart) throw art::Exception(art::errors::LogicError) << "GEANT particle #" << iPart << " returned a null pointer";
  
    if( ! (pPart->Process() == pri) ) continue; // Has to be a primary

    const art::Ptr<simb::MCTruth> & mc_truth = bt->ParticleToMCTruth(pPart);   
    if( ! (mc_truth->Origin() == isGENIE) ) continue; // Has to come from GENIE interaction

    if ( ! (pPart->PdgCode() == isMuonPlus) ) continue; // Has to be a muon

    TVector3 g4parStart (pPart->Vx(), pPart->Vy(), pPart->Vz());
    TVector3 g4parEnd   (pPart->EndPosition()[0], pPart->EndPosition()[1], pPart->EndPosition()[2]);

    _g4muonsForThisEvent++;
    _g4muon_mom = pPart->Momentum().Vect().Mag();
    _g4muon_trklen = (g4parStart-g4parEnd).Mag();

  } // g4 part loop


  std::cout << "nue    " << mclist[iList]->GetNeutrino().Nu().E()
            << "nuvtxx " << mclist[iList]->GetNeutrino().Nu().Vx()
            << "nuvtxy " << mclist[iList]->GetNeutrino().Nu().Vy()
            << "nuvtxz " << mclist[iList]->GetNeutrino().Nu().Vz()
            << std::endl;

  // Verify if true vtx is in fiducial volume
  _trueVtxInFV = (InFV(mcNuVertex.X(), mcNuVertex.Y(), mcNuVertex.Z()) ? 1 : 0)  ;





  //***********************************
  //
  // Pandora particle loop
  //
  //***********************************

  bool recoNuVtxExists = false;

  for (unsigned int n = 0; n < pfParticleList.size(); ++n) {
    const art::Ptr<recob::PFParticle> particle = pfParticleList.at(n);

    // For each PFParticle, get the tracks associated
    lar_pandora::PFParticlesToTracks::const_iterator trackMapIter = pfParticleToTrackMap.find(particle);
    if (trackMapIter != pfParticleToTrackMap.end()) {
      lar_pandora::TrackVector tracks = trackMapIter->second;

    }

    if(lar_pandora::LArPandoraHelper::IsNeutrino(particle)) {
      nPFPhierarchy_perEvent++;
      const std::vector<size_t> &daughterIDs = particle->Daughters();
      lar_pandora::PFParticlesToVertices::const_iterator vertexMapIter = pfParticleToVertexMap.find(particle);
      if (vertexMapIter != pfParticleToVertexMap.end()) {
        lar_pandora::VertexVector vertices = vertexMapIter->second;

        std::cout << "Neutrino has ID: " << pfParticleList.at(n)->Self()
                  << " and " << daughterIDs.size() << " daughters"
                  << " and n vertices " << vertices.size() << std::endl;

        art::Ptr<recob::Vertex> nu_reco_vtx = vertices.at(0);
        double nu_reco_vtx_xyz[3];
        nu_reco_vtx->XYZ(nu_reco_vtx_xyz);


/*
        std::cout << "Nu reco vertex: (x,y,z) = (" << nu_reco_vtx_xyz[0] << " , " << nu_reco_vtx_xyz[1] << " , " << nu_reco_vtx_xyz[2] << ")" << std::endl;
        std::cout << "mclist.size() " << mclist.size() << std::endl;
        std::cout << "True vertex: " << mclist[iList]->GetNeutrino().Nu().Vx() 
                                     << " " << mclist[iList]->GetNeutrino().Nu().Vy()
                                     << " " << mclist[iList]->GetNeutrino().Nu().Vz()
                                     << std::endl;
*/

        // The reco nu vertex is:
        TVector3 recoNuVertex (nu_reco_vtx_xyz[0],nu_reco_vtx_xyz[1],nu_reco_vtx_xyz[2]);

        _nPFP_nu = _nPFP_bkg = -999;

        // Create 2d vectors
        TVector3 recoNuVertex2D (0., recoNuVertex.Y(), recoNuVertex.Z());
        TVector3 mcNuVertex2D   (0., mcNuVertex.Y(), mcNuVertex.Z());
        if((mcNuVertex2D-recoNuVertex2D).Mag() < 10. /*cm*/) {
          _nPFP_nu  = daughterIDs.size();
          recoNuVtxExists = true;
        }
        else
          _nPFP_bkg = daughterIDs.size();

        _run    = e.id().run();
        _subrun = e.id().subRun();
        _event  = e.id().event();

        _tree1->Fill();


        for (unsigned int m = 0; m < daughterIDs.size(); ++m) {
          const art::Ptr<recob::PFParticle> daughter = pfParticleList.at(daughterIDs.at(m));
          std::cout << "Neutrino daughter ID: " << daughterIDs[m]
                    << " IsTrack? " << lar_pandora::LArPandoraHelper::IsTrack(daughter)
                    << " IsShower? " << lar_pandora::LArPandoraHelper::IsShower(daughter)
                    << std::endl;
        }
      }
    }
  }


  //***********************************
  //
  // Save to file
  // 
  // ***********************************

  _run    = e.id().run();
  _subrun = e.id().subRun();
  _event  = e.id().event();
        
  _nPFPhierarchy = nPFPhierarchy_perEvent;
  _recoNuVtxExists = (recoNuVtxExists ? 1 : 0);

  _tree2->Fill();


  // Implementation of required member function here.
}

bool TPCObjectCreator::InFV(double x, double y, double z) {

  //This defines our current settings for the fiducial volume
  double FVx = 256.35;
  double FVy = 233;
  double FVz = 1036.8;
  double borderx = 10.;
  double bordery = 20.;
  double borderz = 10.;

  if(x < (FVx - borderx) && (x > borderx) && (y < (FVy/2. - bordery)) && (y > (-FVy/2. + bordery)) && (z < (FVz - borderz)) && (z > borderz)) return true;
  return false;
}

DEFINE_ART_MODULE(TPCObjectCreator)
