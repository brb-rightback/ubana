#ifndef SINGLE_PHOTON_ANALYSIS
#define SINGLE_PHOTON_ANALYSIS

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"

#include "lardataobj/RecoBase/PFParticle.h"
#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/RecoBase/Shower.h"
#include "lardataobj/RecoBase/Vertex.h"
#include "lardataobj/RecoBase/Cluster.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/SpacePoint.h"
#include "lardataobj/RecoBase/OpFlash.h"
#include "lardataobj/AnalysisBase/Calorimetry.h"
#include "lardataobj/AnalysisBase/BackTrackerMatchingData.h"

#include "larcoreobj/SummaryData/POTSummary.h"

#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCTruth.h"

#include "larpandora/LArPandoraObjects/PFParticleMetadata.h"
#include "larpandora/LArPandoraInterface/LArPandoraHelper.h"

#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "lardata/DetectorInfoServices/LArPropertiesService.h"

#include "larcore/Geometry/Geometry.h"

#include "canvas/Utilities/ensurePointer.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Persistency/Common/FindOneP.h"
#include "canvas/Persistency/Common/FindOne.h"

#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib_except/exception.h"

#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TGraphDelaunay.h"

#include "Pandora/PdgTable.h"
#include <chrono>

#include <iostream>
#include <string>

//------------------------------------------------------------------------------------------------------------------------------------------

namespace single_photon
{
    /**
     *  @brief  SinglePhoton class
     */
    class SinglePhoton : public art::EDAnalyzer
    {
        public:
            typedef art::ValidHandle< std::vector<recob::PFParticle> > PFParticleHandle;
            typedef std::vector< art::Ptr<recob::PFParticle> > PFParticleVector;
            typedef std::vector< art::Ptr<recob::Track> > TrackVector;
          typedef std::vector< art::Ptr<recob::Shower> > ShowerVector;
            typedef std::map< size_t, art::Ptr<recob::PFParticle>> PFParticleIdMap;

            /**
             *  @brief  Constructor
             *
             *  @param  pset the set of input fhicl parameters
             */
            SinglePhoton(fhicl::ParameterSet const &pset);

            /**
             *  @brief  Configure memeber variables using FHiCL parameters
             *
             *  @param  pset the set of input fhicl parameters
             */
            void reconfigure(fhicl::ParameterSet const &pset);

            /**
             *  @brief  Analyze an event!
             *
             *  @param  evt the art event to analyze
             */
            void analyze(const art::Event &evt);

            /**
             *  @brief  Begin the job, setting up !
             *
             */
            void beginJob();

            /**
             *  @brief  End the job, setting down !
             *
             */
            void endJob();


            void beginSubRun(art::SubRun const & sr);

        private:
            void ClearVertex();
            /**
             *  @brief  Produce a mapping from PFParticle ID to the art ptr to the PFParticle itself for fast navigation
             *
             *  @param  pfParticleHandle the handle for the PFParticle collection
             *  @param  pfParticleMap the mapping from ID to PFParticle
             */
            void GetPFParticleIdMap(const PFParticleHandle &pfParticleHandle, PFParticleIdMap &pfParticleMap);

            /**
             * @brief Print out scores in PFParticleMetadata
             *
             * @param evt the art event to analyze
             * @param pfParticleHandle the handle for the PFParticle collection
             */
            void PrintOutScores(const art::Event &evt, const PFParticleHandle &pfParticleHandle) const;

            /**
             *  @brief  Produce a mapping from PFParticle ID to the art ptr to the PFParticle itself for fast navigation
             *
             *  @param  pfParticleMap the mapping from ID to PFParticle
             *  @param  crParticles a vector to hold the top-level PFParticles reconstructed under the cosmic hypothesis
             *  @param  nuParticles a vector to hold the final-states of the reconstruced neutrino
             */
            void GetFinalStatePFParticleVectors(const PFParticleIdMap &pfParticleMap,const lar_pandora::PFParticlesToVertices &particlesToVertices, PFParticleVector &crParticles, PFParticleVector &nuParticles);

            /**
             *  @brief  Collect associated tracks and showers to particles in an input particle vector
             *
             *  @param  particles a vector holding PFParticles from which to find the associated tracks and showers
             *  @param  pfParticleHandle the handle for the PFParticle collection
             *  @param  evt the art event to analyze
             *  @param  tracks a vector to hold the associated tracks
             *  @param  showers a vector to hold the associated showers
             */
            void CollectTracksAndShowers(const PFParticleVector &particles, const PFParticleHandle &pfParticleHandle, const art::Event &evt, TrackVector &tracks, ShowerVector &showers,  std::map< art::Ptr<recob::Track> , art::Ptr<recob::PFParticle>>  &trackToNuPFParticleMap, std::map< art::Ptr<recob::Shower> , art::Ptr<recob::PFParticle>> &showerToNuPFParticleMap );

            void GetVertex(const lar_pandora::PFParticlesToVertices & particlestoVertices, const art::Ptr<recob::PFParticle> & particle );

            void CollectCalo(const art::Event &evt,const art::Ptr<recob::Track> &track);
            void CollectCalo(const art::Event &evt,const art::Ptr<recob::Shower> &shower);



            //----------------  Flashes ----------------------------
            void AnalyzeFlashes(const std::vector<art::Ptr<recob::OpFlash>>& flashes);
            void ClearFlashes();
            void ResizeFlashes(size_t);
            void CreateFlashBranches();

            //----------------  Tracks ----------------------------
            void AnalyzeTracks(const std::vector<art::Ptr<recob::Track>>& tracks, std::map<art::Ptr<recob::Track>, art::Ptr<recob::PFParticle>> & tracktopfparticlemap, std::map<art::Ptr<recob::PFParticle>, std::vector<art::Ptr<recob::SpacePoint>>> & pfparticletospacepointmap );
            void ClearTracks();
            void ResizeTracks(size_t);
            void CreateTrackBranches();

            TGraph proton_length2energy_tgraph;
            double dist_line_point( std::vector<double>&X1, std::vector<double>& X2, std::vector<double>& X0);

            //----------------  Showers ----------------------------

            void AnalyzeShowers(const std::vector<art::Ptr<recob::Shower>>& showers, std::map<art::Ptr<recob::Shower>, art::Ptr<recob::PFParticle> > & showertopfparticlemap, std::map<art::Ptr<recob::PFParticle>, std::vector<art::Ptr<recob::Hit>> > & pfparticletohitmap);
            void ClearShowers();
            void ResizeShowers(size_t);
            void CreateShowerBranches();

            //------------------ Delaunay triangle tools -----------//

            double triangle_area(double a1, double a2, double b1, double b2, double c1, double c2);
            int quick_delaunay_fit(int n, double *X, double *Y, int *num_triangles, double * area);
            int delaunay_hit_wrapper(const std::vector<art::Ptr<recob::Hit>>& hits, std::vector<int> & num_hits, std::vector<int>& num_triangles, std::vector<double> & area);



            std::string m_pandoraLabel;         ///< The label for the pandora producer
            std::string m_trackLabel;           ///< The label for the track producer from PFParticles
            std::string m_showerLabel;          ///< The label for the shower producer from PFParticles
            std::string m_caloLabel;            ///< The label for calorimetry associations producer
            std::string m_flashLabel;
            std::string m_geantModuleLabel;
            std::string m_backtrackerLabel;
            std::string m_hitfinderLabel;
            std::string m_hitMCParticleAssnsLabel;
            std::string m_potLabel;

            bool m_useModBox;
            bool        m_printOutScores;       ///< Option to investigate the associations to scores for PFParticles

            bool m_is_verbose;

            detinfo::DetectorProperties const* theDetector;//



            TTree* pot_tree;
            TTree* vertex_tree;

            //------------ POT related variables --------------
            int m_number_of_events;
            double m_pot_count;
            int m_number_of_vertices;

            //------------ Event Related Variables -------------
            int m_run_number;
            int m_event_number;

            //------------ Vertex Related variables -------------
            double m_vertex_pos_x;
            double m_vertex_pos_y;
            double m_vertex_pos_z;
            int m_reco_asso_showers;

            //-------------- Flash related variables -------------
            std::vector<double> m_reco_flash_total_pe;
            std::vector<double> m_reco_flash_time;
            std::vector<double> m_reco_flash_time_width;
            std::vector<double> m_reco_flash_abs_time;
            std::vector<int>    m_reco_flash_frame;
            std::vector<double> m_reco_flash_ycenter;
            std::vector<double> m_reco_flash_ywidth;
            std::vector<double> m_reco_flash_zcenter;
            std::vector<double> m_reco_flash_zwidth;
            std::vector<double> m_reco_flash_total_pe_in_beamgate;
            std::vector<double> m_reco_flash_time_in_beamgate;
            std::vector<double> m_reco_flash_ycenter_in_beamgate;
            std::vector<double> m_reco_flash_zcenter_in_beamgate;

            int m_reco_num_flashes;
            int m_reco_num_flashes_in_beamgate;

            double m_beamgate_flash_start;
            double m_beamgate_flash_end;

            //------------ Track related Variables -------------
            int m_reco_asso_tracks;
            std::vector<double> m_reco_track_length;
            std::vector<double> m_reco_track_dirx;
            std::vector<double> m_reco_track_diry;
            std::vector<double> m_reco_track_dirz;
            std::vector<double> m_reco_track_startx;
            std::vector<double> m_reco_track_starty;
            std::vector<double> m_reco_track_startz;
            std::vector<double> m_reco_track_endx;
            std::vector<double> m_reco_track_endy;
            std::vector<double> m_reco_track_endz;
            std::vector<double>   m_reco_track_theta_yz;
            std::vector<double>   m_reco_track_phi_yx;


            std::vector<int> m_reco_track_num_trajpoints;
            std::vector<int> m_reco_track_num_spacepoints;
            std::vector<double> m_reco_track_proton_kinetic_energy;
            std::vector<double> m_reco_track_spacepoint_principal0;
            std::vector<double> m_reco_track_spacepoint_principal1;
            std::vector<double> m_reco_track_spacepoint_principal2;

            std::vector<double> m_reco_track_spacepoint_chi;
            std::vector<double> m_reco_track_spacepoint_max_dist;
            //------------ Shower related Variables  -------------

            std::vector<double>   m_reco_shower_startx;
            std::vector<double>   m_reco_shower_starty;
            std::vector<double>   m_reco_shower_startz;
            std::vector<double>   m_reco_shower_dirx;
            std::vector<double>   m_reco_shower_diry;
            std::vector<double>   m_reco_shower_dirz;
            std::vector<double>   m_reco_shower_theta_yz;
            std::vector<double>   m_reco_shower_phi_yx;

            std::vector<double> m_reco_shower_openingangle;
            std::vector<double> m_reco_shower_length;
            std::vector<double> m_reco_shower_conversion_distance;

            std::vector<int> m_reco_shower_delaunay_num_triangles_plane0;
            std::vector<int> m_reco_shower_delaunay_num_triangles_plane1;
            std::vector<int> m_reco_shower_delaunay_num_triangles_plane2;

            std::vector<int> m_reco_shower_num_hits_plane0;
            std::vector<int> m_reco_shower_num_hits_plane1;
            std::vector<int> m_reco_shower_num_hits_plane2;


            std::vector<double> m_reco_shower_delaunay_area_plane0;
            std::vector<double> m_reco_shower_delaunay_area_plane1;
            std::vector<double> m_reco_shower_delaunay_area_plane2;




    };

    DEFINE_ART_MODULE(SinglePhoton)

} // namespace lar_pandora
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
// implementation follows
