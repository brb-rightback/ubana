#include "SinglePhoton_module.h"

namespace single_photon
{
    void SinglePhoton::ClearMCTruths(){
        m_mctruth_num = 0;
        m_mctruth_origin = -99;
        m_mctruth_mode = -99;
        m_mctruth_interaction_type = -99;
        m_mctruth_ccnc = -99;
        m_mctruth_qsqr = -99;
        m_mctruth_nu_E = -99;
        m_mctruth_nu_pdg = 0;
        m_mctruth_lepton_pdg = 0;
        m_mctruth_num_daughter_particles = -99;
        m_mctruth_daughters_pdg.clear();
        m_mctruth_daughters_E.clear();

        m_mctruth_num_exiting_photons =0;
        m_mctruth_num_exiting_protons =0;
        m_mctruth_num_exiting_pi0 =0;
        m_mctruth_num_exiting_pipm =0;
        m_mctruth_num_exiting_neutrons=0;
        m_mctruth_num_exiting_delta0=0;
        m_mctruth_num_exiting_deltapm=0;
        m_mctruth_num_exiting_deltapp=0;

        m_mctruth_exiting_pi0_E.clear();
        m_mctruth_exiting_pi0_px.clear();
        m_mctruth_exiting_pi0_py.clear();
        m_mctruth_exiting_pi0_pz.clear();
    }

    void SinglePhoton::ResizeMCTruths(size_t size){
        m_mctruth_daughters_pdg.resize(size);
        m_mctruth_daughters_E.resize(size);

    }


    void SinglePhoton::CreateMCTruthBranches(){
        vertex_tree->Branch("mctruth_num",&m_mctruth_num);
        vertex_tree->Branch("mctruth_origin",&m_mctruth_origin);
        vertex_tree->Branch("mctruth_nu_pdg",&m_mctruth_nu_pdg);
        vertex_tree->Branch("mctruth_nu_E",&m_mctruth_nu_E);
        vertex_tree->Branch("mctruth_lepton_pdg",&m_mctruth_lepton_pdg);
        vertex_tree->Branch("mctruth_lepton_E",&m_mctruth_lepton_E);
        vertex_tree->Branch("mctruth_mode",&m_mctruth_mode);
        vertex_tree->Branch("mctruth_qsqr",&m_mctruth_qsqr);
        vertex_tree->Branch("mctruth_ccnc",&m_mctruth_ccnc);
        vertex_tree->Branch("mctruth_interaction_type",&m_mctruth_interaction_type);
        
        vertex_tree->Branch("mctruth_num_daughter_particles",&m_mctruth_num_daughter_particles);
        vertex_tree->Branch("mctruth_daughters_pdg",&m_mctruth_daughters_pdg);
        vertex_tree->Branch("mctruth_daughters_E",&m_mctruth_daughters_E);

        vertex_tree->Branch("mctruth_num_exiting_protons",&m_mctruth_num_exiting_protons);
        vertex_tree->Branch("mctruth_num_exiting_photons",&m_mctruth_num_exiting_photons);
        vertex_tree->Branch("mctruth_num_exiting_neutrons",&m_mctruth_num_exiting_neutrons);
        vertex_tree->Branch("mctruth_num_exiting_pi0",&m_mctruth_num_exiting_pi0);
        vertex_tree->Branch("mctruth_num_exiting_pipm",&m_mctruth_num_exiting_pipm);
        vertex_tree->Branch("mctruth_num_exiting_delta0",&m_mctruth_num_exiting_delta0);
        vertex_tree->Branch("mctruth_num_exiting_deltapm",&m_mctruth_num_exiting_deltapm);
        vertex_tree->Branch("mctruth_num_exiting_deltapp",&m_mctruth_num_exiting_deltapp);

        vertex_tree->Branch("mctruth_exiting_pi0_E",&m_mctruth_exiting_pi0_E);
        vertex_tree->Branch("mctruth_exiting_pi0_px",&m_mctruth_exiting_pi0_px);
        vertex_tree->Branch("mctruth_exiting_pi0_py",&m_mctruth_exiting_pi0_py);
        vertex_tree->Branch("mctruth_exiting_pi0_pz",&m_mctruth_exiting_pi0_pz);
    }

    void SinglePhoton::AnalyzeMCTruths(std::vector<art::Ptr<simb::MCTruth>> & mcTruthVector ){
        m_mctruth_num = mcTruthVector.size();
        this->ResizeMCTruths(m_mctruth_num);
    
        if(m_is_verbose) std::cout<<"SinglePhoton::AnalyzeMCTruths()\t||\t Starting to analyze "<<m_mctruth_num<<" simb::MCTruth's."<<std::endl;
        if(m_mctruth_num >1){
            std::cout<<"SinglePhoton::AnalyzeMCTruths()\t||\t WARNING There is more than 1 MCTruth neutrino interaction. Just running over the first simb::MCTruth."<<std::endl;
        }

        for(int i=0; i<std::min(1,m_mctruth_num); i++){
            const art::Ptr<simb::MCTruth> truth = mcTruthVector[i];
            
            m_mctruth_origin = truth->Origin();
            m_mctruth_ccnc = truth->GetNeutrino().CCNC();
            m_mctruth_mode = truth->GetNeutrino().Mode();
            m_mctruth_interaction_type = truth->GetNeutrino().InteractionType();
            m_mctruth_qsqr = truth->GetNeutrino().QSqr();
            m_mctruth_nu_pdg = truth->GetNeutrino().Nu().PdgCode();
            m_mctruth_nu_E = truth->GetNeutrino().Nu().E();
            m_mctruth_lepton_pdg = truth->GetNeutrino().Lepton().PdgCode();
            m_mctruth_lepton_E = truth->GetNeutrino().Lepton().E();
            
            m_mctruth_num_daughter_particles = truth->NParticles();
            this->ResizeMCTruths(m_mctruth_num_daughter_particles);

            for(int j=0; j< m_mctruth_num_daughter_particles; j++){
                    const simb::MCParticle par = truth->GetParticle(j);
                    m_mctruth_daughters_pdg[j] = par.PdgCode();
                    m_mctruth_daughters_E[j] = par.E();
                    
                    switch(m_mctruth_daughters_pdg[j]){
                        case(22):
                                m_mctruth_num_exiting_photons++;
                                break;
                        case(111):
                                m_mctruth_exiting_pi0_E.push_back(par.E());
                                m_mctruth_exiting_pi0_px.push_back(par.Px());
                                m_mctruth_exiting_pi0_py.push_back(par.Py());
                                m_mctruth_exiting_pi0_pz.push_back(par.Pz());
                                m_mctruth_num_exiting_pi0++;
                                break;
                        case(211):
                        case(-211):
                                m_mctruth_num_exiting_pipm++;
                                break;
                        case(2212): 
                                m_mctruth_num_exiting_protons++;
                                break;
                        case(2112): 
                                m_mctruth_num_exiting_neutrons++;
                                break;
                        case(2224):
                                m_mctruth_num_exiting_deltapp++;
                                break;
                        case(2214):
                        case(1114):
                                m_mctruth_num_exiting_deltapm++;
                                break;
                        case(2114):
                                m_mctruth_num_exiting_delta0++;
                                break;
                        default:
                            break;
                    }
            }

        }
    
        if(m_is_verbose){
            std::cout<<"SinglePhoton::AnalyzeMCTruths()\t||\t This is a CCNC: "<<m_mctruth_ccnc<<" event with a nu_pdg: "<<m_mctruth_nu_pdg<<" and "<<m_mctruth_num_daughter_particles<<" exiting particles."<<std::endl;
            std::cout<<"SinglePhoton::AnalyzeMCTruths()\t||\t With  "<<m_mctruth_num_exiting_pi0<<" Pi0, "<<m_mctruth_num_exiting_pipm<<" Pi+/-, "<<m_mctruth_num_exiting_protons<<" Protons, "<<m_mctruth_num_exiting_neutrons<<" neutrons and "<<m_mctruth_num_exiting_delta0<<" delta0, "<<m_mctruth_num_exiting_deltapm<<" deltapm, "<<m_mctruth_num_exiting_deltapp<<" Deltas++"<<std::endl;
        }

    }
}
