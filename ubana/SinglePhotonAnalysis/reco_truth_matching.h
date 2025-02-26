#include "SinglePhoton_module.h"
#include <climits>
//#include <typeinfo>

namespace single_photon
{

    //recoMCmatching but specifically for recob::showers
    void SinglePhoton::showerRecoMCmatching(std::vector<art::Ptr<recob::Shower>>& objectVector,
            std::map<art::Ptr<recob::Shower>,art::Ptr<simb::MCParticle>>& objectToMCParticleMap,
            std::map<art::Ptr<recob::Shower>,art::Ptr<recob::PFParticle>>& objectToPFParticleMap,
            std::map<art::Ptr<recob::PFParticle>, std::vector<art::Ptr<recob::Hit>> >& pfParticleToHitsMap,
            art::FindManyP<simb::MCParticle,anab::BackTrackerHitMatchingData>& mcparticles_per_hit,
            std::vector<art::Ptr<simb::MCParticle>>& mcParticleVector,
            std::map< size_t, art::Ptr<recob::PFParticle>> & pfParticleIdMap,
            std::map< int ,art::Ptr<simb::MCParticle> >  &  MCParticleToTrackIdMap ,
            std::map<int, double>& sliceIdToNuScoreMap,
            std::map<art::Ptr<recob::PFParticle>,bool>& PFPToClearCosmicMap,
            std::map<art::Ptr<recob::PFParticle>, int>& PFPToSliceIdMap,
            std::map<art::Ptr<recob::PFParticle>,bool>& PFPToNuSliceMap){

        std::vector<double> vec_fraction_matched;
        std::map<std::string,bool> map_is_shower_process = {{"compt",true},{"FastScintillation",true},{"eBrem",true},{"phot",true},{"eIoni",true},{"conv",true},{"annihil",true}};
        bool reco_verbose = false;
        
        if(reco_verbose) std::cout<<"Strting "<<objectVector.size()<<" things"<<std::endl;

        //for each recob::track/shower in the event
        for(size_t i=0; i<objectVector.size();++i){
            auto object = objectVector[i];

            //get the associated reco PFP
            if(reco_verbose)std::cout<<"We have "<<objectToPFParticleMap.count(object)<<" matches in map"<<std::endl;
            const art::Ptr<recob::PFParticle> pfp = objectToPFParticleMap[object];

            if (reco_verbose) std::cout<<"SinglePhoton::recoMC()\t||\t looking for a shower match to pfp "<< pfp->Self()<<" which is in slice "<<  PFPToSliceIdMap[pfp]<<std::endl;

            //this was some checks on the PFP side
            /*
            //check to see where the PFP is in the chain
            // If the particle is primary, it doesn't have a parent
            if (pfp->IsPrimary()){std::cout<<"this is the primary particle"<<std::endl;}
            //else get the parent
            else{
            const auto parentIterator = pfParticleIdMap.find(pfp->Parent());
            if (parentIterator == pfParticleIdMap.end()){
            std::cout<<"error: no parent but not primary"<<std::endl;
            }

            const int parentPDG = parentIterator->second->PdgCode();
            std::cout<<"the parent pdg code is "<<parentPDG<<std::endl;

            auto daughers_vec = pfp->Daughters();
            //std::cout<<"the number of daugter particles is "<<daughers_vec.size() <<std::endl;

            for (auto const daughterpfp: daughers_vec){
            const auto daughterIterator = pfParticleIdMap.find(daughterpfp);
            if (daughterIterator == pfParticleIdMap.end()){
            //       std::cout<<"error: didn't find that daughter"<<std::endl;
            } else {
            art::Ptr<recob::PFParticle> daughters = daughterIterator->second;
            //       std::cout<<"the daughter pdg code is "<<daughters->PdgCode()<<std::endl;
            }
            }               

            //const int parentPDG = parentIterator->second->PdgCode();
            //std::cout<<"the parent pdg code is "<<parentPDG<<std::endl;
            }
            */

            //putting in the PFP pdg code as a check

            //and get the hits associated to the reco PFP
            std::vector< art::Ptr<recob::Hit> > obj_hits_ptrs = pfParticleToHitsMap[pfp];

            /**
             * 
             * Loop over hits associated with the reco PFP to find MCParticles which contribute energy to the reco shower
             *
             **/

            std::unordered_map<int,double> objide; //map between the MCParticle track ID and the backtracker energy

            //energy for an MCParticle that comprises the most energy when sum over associated hits in PFP
            //total energy of the reco PFP taken from the sum of the hits associated to an MCParticle
            //  tote used out of this scope???
            // double maxe=-1, tote=0;                // tote unused
            double maxe=-1;

            std::vector<double> total_energy_on_plane = {0.0,0.0,0.0};
            //simb::MCParticle const * best_matched_mcparticle = NULL; //pointer for the particle match we will calculate
            art::Ptr<simb::MCParticle> best_matched_mcparticle; //pointer for the MCParticle match we will calculate

            //    std::vector<simb::MCParticle const *> particle_vec;
            //    std::vector<anab::BackTrackerHitMatchingData const *> match_vec;

            std::vector<art::Ptr<simb::MCParticle>> particle_vec; //vector of all MCParticles associated with a given hit in the reco PFP
            std::vector<anab::BackTrackerHitMatchingData const *> match_vec; //vector of some backtracker thing

            int n_associated_mcparticle_hits = 0;
            int n_not_associated_hits = 0;

            //this is the vector that will store the associated MC paritcles, as well as a MAP to the amount of energy associated
            std::vector<art::Ptr<simb::MCParticle>> asso_mcparticles_vec;
            std::map<art::Ptr<simb::MCParticle>, std::vector<double>> map_asso_mcparticles_energy;

            bool found_a_match = false;

            //std::cout<<"SinglePhoton::RecoMC()\t||\t On object: "<<i<<" with pfp "<< pfp->Self() <<"and slice id "<<PFPToSliceIdMap[pfp]<<". This object has "<<obj_hits_ptrs.size()<<" hits associated with it"<<std::endl;

            //loop only over hits associated to this reco PFP
            for(size_t i_h=0; i_h < obj_hits_ptrs.size(); ++i_h){

                int which_plane = (int)obj_hits_ptrs[i_h]->View();

                particle_vec.clear(); match_vec.clear(); //only store per hit

                //for the hit, fill the backtracker info 
                mcparticles_per_hit.get(obj_hits_ptrs[i_h].key(), particle_vec, match_vec);
               // std::cout<<"for hit "<< i_h <<" particle_vec.size() = "<< particle_vec.size()<< " and match_vec.size() = "<< match_vec.size()<<std::endl; 
                 

                //mcparticles_per_hit.get(obj_hits_ptrs[i_h].key(),particle_vec,match_vec);
                //the .key() gives us the index in the original collection
                //std::cout<<"REC: hit "<<i_h<<" has "<<particle_vec.size()<<" MCparticles assocaied: "<<std::endl;

                //if there is an MCParticle associated to this hit
                if(particle_vec.size()>0) n_associated_mcparticle_hits++;

                if(particle_vec.size()==0) n_not_associated_hits++;



                //for each MCParticle associated with this hit
                for(size_t i_p=0; i_p<particle_vec.size(); ++i_p){
                    //add the energy of the back tracked hit for this MCParticle to the track id for the MCParticle in the map
                    objide[ particle_vec[i_p]->TrackId()] += match_vec[i_p]->energy; //store energy per track id

                    //if the id isn't already in the map, store it in the vector of all associated MCParticles
                    if(std::find(asso_mcparticles_vec.begin(), asso_mcparticles_vec.end(),  particle_vec[i_p]) == asso_mcparticles_vec.end()){
                        asso_mcparticles_vec.push_back(particle_vec[i_p]);
                        map_asso_mcparticles_energy[particle_vec[i_p]] = {0.0,0.0,0.0};
                        map_asso_mcparticles_energy[particle_vec[i_p]][which_plane] =  match_vec[i_p]->energy;
                    }else{
                        map_asso_mcparticles_energy[particle_vec[i_p]][which_plane] += match_vec[i_p]->energy;
                    }

                    //add the energy of the back tracked hit to the total energy for the PFP
                    // tote += match_vec[i_p]->energy; //calculate total energy deposited // unused
                    total_energy_on_plane[which_plane]+=match_vec[i_p]->energy;


                    //want the MCParticle with the max total energy summed from the back tracker hit energy from hits in PFP
                    //TODO: this part will change once the parts below are fully implemented
                    if( objide[ particle_vec[i_p]->TrackId()] > maxe ){ //keep track of maximum
                        maxe = objide[ particle_vec[i_p]->TrackId() ];
                        best_matched_mcparticle = particle_vec[i_p]; //we will now define the best match as a source MCP rather than the max single energy contributor 
                        found_a_match = true;//will be false for showers from overlay
                    }
                }//end loop over particles per hit


            } // end loop over hits

            double fraction_num_hits_overlay = (double)n_not_associated_hits/(double)obj_hits_ptrs.size();

            if(reco_verbose)std::cout << "SinglePhoton::recoMC()\t||\t On Object "<<i<<". The number of MCParticles associated with this PFP is "<<objide.size()<<std::endl;       
            if(reco_verbose) std::cout<<"SinglePhoton::recoMC()\t||\t the fraction of hits from overlay is is "<<fraction_num_hits_overlay<<" ("<<n_not_associated_hits<<"/"<<obj_hits_ptrs.size()<<")"<<std::endl;


            if(n_associated_mcparticle_hits == 0){
                //This will only occur if the whole recob::PFParticle is PURELY associated with an overlay object
                found_a_match =false;
                if(!found_a_match){
                }
                //Here we will fill every sim_shower_XXX variable with -999 or something like that 

                m_sim_shower_matched[i] = 0;
                m_sim_shower_energy[i] = -999;
                m_sim_shower_mass[i] = -999;
                m_sim_shower_kinetic_energy[i] = -999;
                m_sim_shower_pdg[i] = -999;
                m_sim_shower_trackID[i] = -999;
                m_sim_shower_process[i] = "overlay";
                m_sim_shower_end_process[i] = "overlay";
                m_sim_shower_parent_pdg[i] = -999;
                m_sim_shower_parent_trackID[i] = -999;
                m_sim_shower_vertex_x[i] = -9999;
                m_sim_shower_vertex_y[i] = -9999;
                m_sim_shower_vertex_z[i] = -9999;

                m_sim_shower_start_x[i] = -9999;
                m_sim_shower_start_y[i] = -9999;
                m_sim_shower_start_z[i] = -9999;
                m_sim_shower_px[i] = -9999;
                m_sim_shower_py[i] = -9999;
                m_sim_shower_pz[i] = -9999;

                m_sim_shower_is_true_shower[i] = -999;
                m_sim_shower_best_matched_plane[i] = -999;
                m_sim_shower_matched_energy_fraction_plane0[i] = -999;
                m_sim_shower_matched_energy_fraction_plane1[i] = -999;
                m_sim_shower_matched_energy_fraction_plane2[i] = -999;

                m_sim_shower_overlay_fraction[i] = fraction_num_hits_overlay;
                m_sim_shower_sliceId[i] = -999;
                m_sim_shower_nuscore[i] = -999;
                m_sim_shower_isclearcosmic[i] = -999;
                m_sim_shower_is_nuslice[i] = -999;


                continue;
            }//


	    /*  ********** if shower has been matched to MCParticle ************************* */

            /*
             *
             * Loop over each MCParticle associated to the reco shower to find the source particle
             *
             */

            std::map<int, art::Ptr<simb::MCParticle>> mother_MCP_map; //map between MCP track id and the source MCP

            std::vector<art::Ptr<simb::MCParticle>> marks_mother_vector;  // a vector of mother MCP
            std::map<art::Ptr<simb::MCParticle>, std::vector<double>> marks_mother_energy_fraction_map; // map of mother MCP and its energy on 3 planes

            int this_mcp_id = -1; //the track id for the current MCP in parent tree
            int last_mcp_id = -1; //the track id for the previous MCP in parent tree
            int i_mcp = 0;

            int num_bt_mothers =0;  // number of associated MCP that has mothers

            //reco_verbose = false;
            //for each MCP that's associated to the reco shower
            for(auto mcp:asso_mcparticles_vec){

                if(reco_verbose) std::cout<<"-----------------------------Start L1 Loop --------------------------------------------------"<<std::endl;
                if(reco_verbose) std::cout<<"L1: ("<<i<<" <-> "<<i_mcp<<") Start by Looking at an MCP with pdg code "<<mcp->PdgCode()<<" and status code "<<mcp->StatusCode()<<" TrackID: "<<mcp->TrackId()<<std::endl;
                if(reco_verbose) std::cout<<"L1: ("<<i<<" <-> "<<i_mcp<<") This MCP gave "<<   map_asso_mcparticles_energy[mcp][0] <<" | "<<map_asso_mcparticles_energy[mcp][1]<<" | "<<map_asso_mcparticles_energy[mcp][2]<<" energy to the recob::Object on each plane"<<std::endl;
                //                std::cout<<"L1: the mother of this MCP is track id "<<mcp->Mother()<<" and there are "<<mcp->NumberDaughters()<<" daughters"<<std::endl;

                //get the track ID for the current MCP
                this_mcp_id = mcp->TrackId();
                last_mcp_id = this_mcp_id;//initialize the previous one

                //while the track id is valid, move up the parent tree for the MCP that contributes to the reco shower
                //currently it keeps going until it hits the top of the interaction chain, but this is likely too far
                //to do a proper match you need to check for different cases and stop once one is fulfilled
                while(this_mcp_id >= 0 ){                  
                    art::Ptr<simb::MCParticle> this_mcp = MCParticleToTrackIdMap[this_mcp_id];//get the MCP associated to the track ID
                    // std::cout<<"going up the tree got mother particle"<<std::endl;

                    //check if it's a valid MCP
                    if (this_mcp.isNull()){
                        if(reco_verbose)   std::cout<<"L1: ("<<i<<" <-> "<<i_mcp<<")  null pointer at id "<<this_mcp_id<<std::endl;
                        this_mcp_id = last_mcp_id; //if invalid, move back a level to the previous MCP in parent tree and break the loop
                        break;
                    }

                    //If primary particle will have process "primary"
                    if(reco_verbose)    std::cout<<"L1: ("<<i<<" <-> "<<i_mcp<<")  going up the tree at an MCP with track id  "<<this_mcp_id<<", pdg code "<<this_mcp->PdgCode()<<", and status code "<<this_mcp->StatusCode()<<" and Mother: "<<this_mcp->Mother()<<" Process: "<<this_mcp->Process()<<" EndProcess: "<<this_mcp->EndProcess()<<std::endl;

                    //if it is a valid particle, iterate forward to the mother
                    last_mcp_id = this_mcp_id;
                    this_mcp_id =  this_mcp->Mother();

                    //Check to see if this MCP was created in a "showery" process
                    if(map_is_shower_process.count(this_mcp->Process()) > 0){
                        //if it was, keep going, 

                    }else if(this_mcp->Process()=="primary"){
                        //if its primary, great! Note it.
                        if(reco_verbose)  std::cout<<"L1: Backtracked to primary! breaking"<<std::endl;
                        this_mcp_id = last_mcp_id; //if invalid, move back a level to the previous MCP in parent tree and break the loop
                        break;
                    }else{
                        if(reco_verbose) std::cout<<"L1: Backtracked to a particle created in "<<this_mcp->EndProcess()<<"! breaking"<<std::endl;
                        this_mcp_id = last_mcp_id; //if invalid, move back a level to the previous MCP in parent tree and break the loop
                        break;
                    }
                }

                //if the MCP at the top of the interaction chain has a valid track id store this in the mother map
                if (this_mcp_id >= 0){

		    //Guanqun: this line here doesn't really cosider other break cases than finding primary particle
                    if(reco_verbose)   std::cout<<"L1: ("<<i<<" <-> "<<i_mcp<<") Storing the mother mother particle with track id "<<this_mcp_id<<" and pdg code "<<MCParticleToTrackIdMap[this_mcp_id]->PdgCode()<<" and status code "<<MCParticleToTrackIdMap[this_mcp_id]->StatusCode()<<std::endl;

                    mother_MCP_map[this_mcp_id] = MCParticleToTrackIdMap[this_mcp_id];//putting it in a map allows for multiple contributing MCP's in the reco shower to have the same mother MCP

                    bool is_old = false;

                    for(size_t k=0; k< marks_mother_vector.size(); k++){
                        //if its in it before, just run with it
                        if(marks_mother_vector[k]==MCParticleToTrackIdMap[this_mcp_id]){
                            marks_mother_energy_fraction_map[marks_mother_vector[k]][0] += map_asso_mcparticles_energy[mcp][0];
                            marks_mother_energy_fraction_map[marks_mother_vector[k]][1] += map_asso_mcparticles_energy[mcp][1];
                            marks_mother_energy_fraction_map[marks_mother_vector[k]][2] += map_asso_mcparticles_energy[mcp][2];
                            is_old = true;
                            break;
                        }
                    }
                    if(is_old==false){
                        marks_mother_vector.push_back(MCParticleToTrackIdMap[this_mcp_id]);
                        marks_mother_energy_fraction_map[marks_mother_vector.back()] = {0.0,0.0,0.0};
                        marks_mother_energy_fraction_map[marks_mother_vector.back()][0] =  map_asso_mcparticles_energy[mcp][0];
                        marks_mother_energy_fraction_map[marks_mother_vector.back()][1] =  map_asso_mcparticles_energy[mcp][1];
                        marks_mother_energy_fraction_map[marks_mother_vector.back()][2] =  map_asso_mcparticles_energy[mcp][2];
                    }


                    num_bt_mothers++;
                } else{
                    if(reco_verbose)  std::cout<<"L1: error, the mother mother id was "<<this_mcp_id <<std::endl;

                }

                if(reco_verbose)  std::cout<<"-----------------------------End L1 Loop --------------------------------------------------"<<std::endl;
                i_mcp++;
            }//for each MCParticle that's associated to a the recob::Shower

            //reco_verbose = true;
            //there should be at least 1 mother MCP
            if(reco_verbose)           std::cout<<"SinglePhoton::recoMC()\t||\t the number of source mother particles is "<<mother_MCP_map.size()<<" of which : "<<marks_mother_vector.size()<<" are unique!"<<std::endl;

            if(reco_verbose)       std::cout<<"---------------------------- L2-------------------------------"<<std::endl;

            double best_mother_index = 0;
            double best_mother_energy = -9999;
            int best_mother_plane = -99;

            for(size_t p=0; p< marks_mother_vector.size(); p++){
                art::Ptr<simb::MCParticle> mother = marks_mother_vector[p];
                std::vector<double> mother_energy_recod = marks_mother_energy_fraction_map[mother];
                if(reco_verbose)    std::cout<<"L2: Mother candidate "<<p<<" TrackID "<<mother->TrackId()<<" Process: "<<mother->Process()<<" EndProcess: "<<mother->EndProcess()<<std::endl;
                if(reco_verbose)   std::cout<<"L2: Mother candidate "<<p<<" Energy "<<mother->E()<<" Reco'd Energy: "<<mother_energy_recod[0]<<" | "<<mother_energy_recod[1]<<" | "<<mother_energy_recod[2]<<" Fraction: ("<<mother_energy_recod[0]/(1000*mother->E())*100.0<<"% , "<<mother_energy_recod[1]/(1000*mother->E())*100.0<<"% , "<<mother_energy_recod[2]/(1000*mother->E())*100.0<<"% )"<<std::endl;

                if( mother_energy_recod[0] > best_mother_energy){
                    best_mother_index = p;
                    best_mother_energy = mother_energy_recod[0];
                    best_mother_plane = 0;
                }

                if( mother_energy_recod[1] > best_mother_energy){
                    best_mother_index = p;
                    best_mother_energy = mother_energy_recod[1];
                    best_mother_plane = 1;
                }

                if( mother_energy_recod[2] > best_mother_energy){
                    best_mother_index = p;
                    best_mother_energy = mother_energy_recod[2];
                    best_mother_plane = 2;
                }

            }

            if(marks_mother_vector.size()!=0){
                //if(reco_verbose)  std::cout<<"SinglePhoton::recoMC()\t||\t The `BEST` mother is a "<<marks_mother_vector[best_mother_index]->PdgCode()<<" at "<<best_mother_index<<" on plane: "<<best_mother_plane<<std::endl;
                std::cout<<"SinglePhoton::recoMC()\t||\t The `BEST` mother is a "<<marks_mother_vector[best_mother_index]->PdgCode()<<" at "<<best_mother_index<<" on plane: "<<best_mother_plane<<std::endl;
                for(int l=0; l<3; l++){
                    std::cout<<"SinglePhoton::recoMC()\t||\t It represents "<<marks_mother_energy_fraction_map[marks_mother_vector[best_mother_index]][l]/total_energy_on_plane[l]*100.0<<"% of the energy on plane: "<<l<<" which is "<<total_energy_on_plane[l] <<std::endl;
                }
            }



	    // now have found the best mother of the shower
            if(reco_verbose) std::cout<<"---------------------------- L2-------------------------------"<<std::endl;
            const art::Ptr<simb::MCParticle> match = marks_mother_vector[best_mother_index];

            std::vector<double> corrected_vertex(3), corrected_start(3);
            this->spacecharge_correction(match, corrected_vertex);


            if(match->PdgCode()==22){ // if it's a gamma
                std::vector<double> tmp  ={match->EndX(), match->EndY(), match->EndZ()};
                this->spacecharge_correction(match, corrected_start, tmp );
                m_sim_shower_is_true_shower[i] = 1;
            }else if(abs(match->PdgCode())==11){  // if it's e+/e-
                this->spacecharge_correction(match, corrected_start);
                m_sim_shower_is_true_shower[i] = 1;
            }else{
                corrected_start = {-999,-999,-999};
                m_sim_shower_is_true_shower[i] = 0;
            }

            art::Ptr<simb::MCParticle> match_mother = MCParticleToTrackIdMap[match->Mother()];

            if (match_mother.isNull()){
                m_sim_shower_parent_pdg[i] = -1;
                m_sim_shower_parent_trackID[i] = -1;

            }else{
                m_sim_shower_parent_pdg[i] = match_mother->PdgCode();
                m_sim_shower_parent_trackID[i] = match_mother->TrackId();
            }



            m_sim_shower_matched[i] = 1;
            m_sim_shower_energy[i] = match->E();
            m_sim_shower_mass[i] = match->Mass();
            m_sim_shower_kinetic_energy[i] = match->E()-match->Mass();
            m_sim_shower_pdg[i] = match->PdgCode();
            m_sim_shower_trackID[i] = match->TrackId();
            m_sim_shower_process[i] = match->Process();
            m_sim_shower_end_process[i] = match->EndProcess();
            m_sim_shower_vertex_x[i] = corrected_vertex[0];
            m_sim_shower_vertex_y[i] = corrected_vertex[1];
            m_sim_shower_vertex_z[i] =corrected_vertex[2];

            m_sim_shower_start_x[i] = corrected_start[0];
            m_sim_shower_start_y[i] = corrected_start[1];
            m_sim_shower_start_z[i] =corrected_start[2];

            m_sim_shower_px[i] = match->Px();
            m_sim_shower_py[i] = match->Py();
            m_sim_shower_pz[i] = match->Pz();

            // should've use 'best_mother_plane' here
	    m_sim_shower_best_matched_plane[i] = best_mother_index;
            m_sim_shower_matched_energy_fraction_plane0[i] = marks_mother_energy_fraction_map[marks_mother_vector[best_mother_index]][0]/total_energy_on_plane[0];
            m_sim_shower_matched_energy_fraction_plane1[i] = marks_mother_energy_fraction_map[marks_mother_vector[best_mother_index]][1]/total_energy_on_plane[1];
            m_sim_shower_matched_energy_fraction_plane2[i] = marks_mother_energy_fraction_map[marks_mother_vector[best_mother_index]][2]/total_energy_on_plane[2];

            m_sim_shower_overlay_fraction[i] = fraction_num_hits_overlay;

            mcParticleVector.push_back(match);
            objectToMCParticleMap[object] = mcParticleVector.back();

            m_sim_shower_sliceId[i] = PFPToSliceIdMap[pfp];
            m_sim_shower_nuscore[i] = sliceIdToNuScoreMap[ m_sim_shower_sliceId[i]] ;
            m_sim_shower_isclearcosmic[i] = PFPToClearCosmicMap[pfp];
            if (m_sim_shower_isclearcosmic[i]== false){
                std::cout<<"sim shower is matched to non-clear cosmic PFP "<<pfp->Self()<<std::endl;
            }
            m_sim_shower_is_nuslice[i] = PFPToNuSliceMap[pfp];

            // if (PFPToNuSliceMap[pfp] ==true){
            //     std::cout<<"m_sim_shower_is_nuslice is true for sim shower matched to reco pfp id "<<pfp->Self()<<std::endl;

            // }
            //
            //  if (m_sim_shower_pdg[i]==22){
          if (reco_verbose)  std::cout<<"looking at pfp "<< pfp->Self()<<" with is matched to true particle with pdg  m_sim_shower_pdg[i]= "<<  m_sim_shower_pdg[i]<< ". is_nuslice = "<< m_sim_shower_is_nuslice[i]<<" in slice "<< m_sim_shower_sliceId[i]<<". The matched energy for this shower from mark's mother particle with pdg "<<marks_mother_vector[best_mother_index]->PdgCode()<< " is "<<m_sim_shower_matched_energy_fraction_plane0[i]<<"/"<<m_sim_shower_matched_energy_fraction_plane1[i]<<"/" <<m_sim_shower_matched_energy_fraction_plane2[i]<<std::endl;
           // std::cout<<"The best plane is "<< best_mother_plane<<std::endl;

            //  }






            //OLD OLD OLD

            /*
             *
             *For each source particle in the event, follow down through daughters to accumulate all the particles below the mother in the parent tree
             *
             */



            /*

            //for each source mother particle, if it's a photon, follow the chain and sum the hits
            std::vector<std::vector<int>> mother_contributing_MCP; //stores all the MCP's in the chain for all mothers-> this can probably be modified to only store ones which contribute to the reco shower
            std::vector<int> all_contributing_MCP; //all the MCP's in the chain for this mother

            //for each of the mother particles
            for(auto pair: mother_MCP_map){
            all_contributing_MCP.clear();
            art::Ptr<simb::MCParticle> particle = pair.second;//get the mother MCP
            all_contributing_MCP.push_back(pair.first);//save the MCP track id 
            int numDaughters = -1;//the number of daughters for the current generation
            std::vector<int> current_ids; //the track id's for the current generation
            std::vector<int> next_ids;//the track id's for the next generation (daughters of current generatiob)

            //std::cout<<"starting from mother particle at head of chain with pdg code "<<particle->PdgCode()<<" and track id "<<pair.first<<std::endl;

            numDaughters = particle->NumberDaughters();//start with the number of daughters for the mother mother particle

            //std::cout<<"this particle has "<<numDaughters<<" daughters"<<std::endl;

            //for each of the particles in the first daughter generation
            for(int i = 0; i < numDaughters; i++){
            int id = particle->Daughter(i); //get the track id of the MCP
            current_ids.push_back(id);//save the id to the list of the current generation
            all_contributing_MCP.push_back(id);//save it to the list of all of the MCP's in the chain
            }


            //while there are more generations of daughter particles (not at the end of the chain)
            while(numDaughters>0){
            //for each MCP in the current generation
            for(int id:current_ids){
            //get the particle and check it's valid
            art::Ptr<simb::MCParticle> particle = MCParticleToTrackIdMap[id];
            if (particle.isNull()) continue;

            //get the number of daughters
            int n = particle->NumberDaughters();

            //loop over the daughters
            for (int i = 0; i < n; i++){
            int daughterId = particle->Daughter(i);

            //save daughters to list of all contributing mcps
            all_contributing_MCP.push_back(daughterId);

            //add daughters to list for next gen
            next_ids.push_back(daughterId);

            }   
            }

            numDaughters = current_ids.size(); //update the number of daughters in the next generation

            //std::cout<<"num daughters after this generation is "<<numDaughters<<std::endl;

            current_ids = next_ids; //update the track id's to the next generation
            next_ids.clear(); //clear the list for the next next generation
            }//while there are further daughters


            //save the vector of MCP's from this mother
            mother_contributing_MCP.push_back(all_contributing_MCP);
            }//for each mother mother particle

            std::cout<<"SinglePhoton::recoMC()\t||\t all candidate MCParticles number is "<<all_contributing_MCP.size()<<std::endl;

            //
            //Compare the list of all of the MCP's from the mother MCP(s) and the list of all MCP's which contribute to the reco shower
            //


            std::vector<int> count_vec(mother_contributing_MCP.size()); //stores the number of MCP's in the chain from each mother which match to the reco shower
            std::vector<double> energy_contributing_MCP(mother_contributing_MCP.size()); //the total energy of all the MCP's in the chain from the mother which contribute to the shower
            //for each MCP from the chain of mother mother particle and daughters, check how much it overlaps with the MCP's that contribute to the shower
            for (unsigned int i = 0; i< mother_contributing_MCP.size(); i++){
                std::vector<int> mcp_vec =  mother_contributing_MCP[i];
                int count = 0;     

                std::cout<<"SinglePhoton::recoMC()\t||\t on mother_contributing_MCP: "<<i<<std::endl;

                for (int track_id:mcp_vec){
                    //check if it's in the map of MCP's in the reco shower
                    auto iter = objide.find(track_id);
                    if (iter != objide.end()){
                        count++;//count the number of MCP

                        //add the energy to the total for this chain
                        energy_contributing_MCP[i] += objide[track_id];
                    }//if the MCP contributes to the shower

                }//for each MCP in the chain from this mother
                count_vec[i] = count;
            }//for each mother/source MCP


            if(count_vec.size()>0){
                //check the total number of contributing MCP
                std::cout<<"SinglePhoton::recoMC()\t||\t the number of MCP associated with the first mother mother particle that also deposit hits in the recob::shower is "<<count_vec[0]<<" and the summed energy is "<<energy_contributing_MCP[0]<<std::endl;  
            }else{
                std::cout<<"SinglePhoton::recoMC()\t||\t Well this failed then."<<std::endl;
            }

            */

        }//end vector loop.

        return ;
    }//end showerRecoMCmatching



    /* @brief: a simpler MCmatching function for track and shower
     * @argument to be filled in function body:
     * 		objectToMCParticleMap: map of object (track, shower) to its best-matching MCParticle
     * 		mcParticleVector: a vector of best-matching MCParticle corresponding to objectVector
     * @return: a vector of fraction number, which is the fraction of unassociated hits in all reco hits of PFParticle
     */	
    //Typenamed for recob::Track and recob::Shower
    template<typename T>
        std::vector<double> recoMCmatching(std::vector<T>& objectVector,
                std::map<T,art::Ptr<simb::MCParticle>>& objectToMCParticleMap,
                std::map<T,art::Ptr<recob::PFParticle>>& objectToPFParticleMap,
                std::map<art::Ptr<recob::PFParticle>, std::vector<art::Ptr<recob::Hit>> >& pfParticleToHitsMap,
                art::FindManyP<simb::MCParticle,anab::BackTrackerHitMatchingData>& mcparticles_per_hit,
                std::vector<art::Ptr<simb::MCParticle>>& mcParticleVector){

            std::vector<double> trk_overlay_vec;
            std::vector<double> vec_fraction_matched;
            bool reco_verbose = false;
            //for each recob::track/shower in the event
            for(size_t i=0; i<objectVector.size();++i){
                auto object = objectVector[i];

                //get the associated reco PFP
                const art::Ptr<recob::PFParticle> pfp = objectToPFParticleMap[object];

                // std::cout<<"SinglePhoton::recoMCmatching()\t||\t looking for a track match to pfp"<< pfp->Self()<<std::endl;


                int pdg = pfp->PdgCode();
                //and get the hits associated to the reco PFP
                std::vector< art::Ptr<recob::Hit> > obj_hits_ptrs = pfParticleToHitsMap[pfp];

                std::unordered_map<int,double> objide; //map between the MCParticle track ID and the backtracker energy

                //energy for an MCParticle that comprises the most energy when sum over associated hits in PFP
                //total energy of the reco PFP taken from the sum of the hits associated to an MCParticle
                double maxe=-1, tote=0;                

                //simb::MCParticle const * best_matched_mcparticle = NULL; //pointer for the particle match we will calculate
                art::Ptr<simb::MCParticle> best_matched_mcparticle; //pointer for the MCParticle match we will calculate

                //    std::vector<simb::MCParticle const *> particle_vec;
                //    std::vector<anab::BackTrackerHitMatchingData const *> match_vec;

                std::vector<art::Ptr<simb::MCParticle>> particle_vec; //vector of all MCParticles associated with a given hit in the reco PFP
                std::vector<anab::BackTrackerHitMatchingData const *> match_vec; //vector of some backtracker thing

                bool found_a_match = false;
                int n_associated_mcparticle_hits = 0;
                int n_not_associated_hits = 0;

            //    std::cout<<"REC: This object with pfp "<< pfp->Self() <<" in slice "<<PFPToSliceIdMap[pfp] <<" has "<<obj_hits_ptrs.size()<<" hits associated with it"<<std::endl;

                //loop only over hits associated to this reco PFP
                for(size_t i_h=0; i_h < obj_hits_ptrs.size(); ++i_h){

                    particle_vec.clear(); match_vec.clear(); //only store per hit

                    //for the hit, fill the backtracker info
                    
                     mcparticles_per_hit.get(obj_hits_ptrs[i_h].key(), particle_vec, match_vec);
                 //    std::cout<<"for hit "<< i_h <<" particle_vec.size() = "<< particle_vec.size()<< " and match_vec.size() = "<< match_vec.size()<<std::endl; 
                 
                    //mcparticles_per_hit.get(obj_hits_ptrs[i_h].key(),particle_vec,match_vec);
                    //the .key() gives us the index in the original collection
                    //std::cout<<"REC: hit "<<i_h<<" has "<<particle_vec.size()<<" MCparticles assocaied: "<<std::endl;

                    //if there is an MCParticle associated to this hit

                    //if there is an MCParticle associated to this hit
                    if(particle_vec.size()>0) n_associated_mcparticle_hits++;

                    if(particle_vec.size()==0) n_not_associated_hits++;


                    //loop over MCparticles finding which is the MCparticle with most "energy" matched correctly
                    //for each MCParticle associated with this hit
                    for(size_t i_p=0; i_p<particle_vec.size(); ++i_p){
                        //add the energy of the back tracked hit for this MCParticle to the track id for the MCParticle in the map
                        objide[ particle_vec[i_p]->TrackId()] += match_vec[i_p]->energy; //store energy per track id

                        //add the energy of the back tracked hit to the total energy for the PFP
                        tote += match_vec[i_p]->energy; //calculate total energy deposited

                        //want the MCParticle with the max total energy summed from the back tracker hit energy from hits in PFP
                        if( objide[ particle_vec[i_p]->TrackId() ] > maxe ){ //keep track of maximum
                            maxe = objide[ particle_vec[i_p]->TrackId() ];
                            best_matched_mcparticle = particle_vec[i_p];
                            found_a_match = true;
                        }
                    }//end loop over particles per hit
                }


                double fraction_num_hits_overlay = (double)n_not_associated_hits/(double)obj_hits_ptrs.size();

                trk_overlay_vec.push_back(fraction_num_hits_overlay);
                if(n_associated_mcparticle_hits == 0){
                    //This will only occur if the whole recob::PFParticle is associated with an overlay object
                    //std::cout<<fraction_num_hits_overlay<<std::endl;
                }//for each recob::track/shower in the event

                //std::cout << "SinglePhoton::recoMC()\t||\t the number of MCParticles associated with this PFP is "<<objide.size()<<std::endl;       

                if(found_a_match){
                    mcParticleVector.push_back(best_matched_mcparticle);
                    objectToMCParticleMap[object] = mcParticleVector.back();
                }else{
                    // mcParticleVector.push_back(0);
                }
                vec_fraction_matched.push_back(maxe/tote);
                // if(m_is_verbose){
                //     std::cout << "SinglePhoton::recoMC()\t||\t the fracrion matched is "<<maxe/tote<<std::endl;
                // }


                if(!found_a_match){
                    if(reco_verbose) std::cout << "SinglePhoton::recoMC()\t||\t NO MATCH NO MATCH (from my loop) for PFP with pdg  "<<pdg<<std::endl;
                    if(reco_verbose)std::cout<<" count "<<objectToMCParticleMap.count(object)<<std::endl;
                }else{
                    //if(reco_verbose)  
                    std::cout << "SinglePhoton::recoMC()\t||\t Final Match (from my loop) for PFP with pdg "<<pdg<<" is " << best_matched_mcparticle->TrackId() << " with energy " << maxe << " over " << tote << " (" << maxe/tote << ")"
                        << " pdg=" << best_matched_mcparticle->PdgCode()
                        << " trkid=" << best_matched_mcparticle->TrackId()
                        << " ke=" << best_matched_mcparticle->E()-best_matched_mcparticle->Mass()<< "\n";
                }

            }//end vector loop.
            //return vec_fraction_matched;
            return trk_overlay_vec;
        }


    //Typenamed for simb::MCTrack and sim::MCShower
    /* @brief: tranverse through mcParticleVector, for each mcParticle, if an mcObject is found with same track ID
     *		put the particle and object in the mcParticleToMCObjectMap as a pair
     */
    template<typename T>
        void perfectRecoMatching(
                std::vector<art::Ptr<simb::MCParticle>>& mcParticleVector,
                std::vector<T>& mcObjectVector,
                std::map<art::Ptr<simb::MCParticle>,T>& mcParticleToMCObjectMap
                ){


            for(size_t io=0; io<mcObjectVector.size(); ++io){

                const T object = mcObjectVector[io];
                int object_trackID =object->TrackID(); 
                int object_mother_trackID =object->MotherTrackID(); 
                int object_ancestor_trackID =object->AncestorTrackID();
                const std::vector< unsigned int > object_daughters_trackID = object->DaughterTrackID();
                std::cout<<"KRANK: "<<io<<" "<<object_trackID<<" "<<object_mother_trackID<<" "<<object_ancestor_trackID<<std::endl;
                std::cout<<"KRANK: "<<object_daughters_trackID.size()<<std::endl;
            }




            return;

            for(size_t ip=0; ip<mcParticleVector.size(); ++ip){
                const art::Ptr<simb::MCParticle> particle = mcParticleVector[ip];
                int particle_trackID = particle->TrackId();

                std::vector<int> id_matches;
                std::vector<int> mother_id_matches;
                std::vector<int> ancestor_id_matches;

                for(size_t io=0; io<mcObjectVector.size(); ++io){

                    const T object = mcObjectVector[io];
                    int object_trackID =object->TrackID(); 
                    int object_mother_trackID =object->MotherTrackID(); 
                    int object_ancestor_trackID =object->AncestorTrackID();

                    if(object_trackID == particle_trackID ) id_matches.push_back(io);
                    if(object_mother_trackID == particle_trackID ) mother_id_matches.push_back(io);
                    if(object_ancestor_trackID == particle_trackID ) ancestor_id_matches.push_back(io);
                }

                int num_id_matches=id_matches.size();
                int num_mother_id_matches=mother_id_matches.size();
                int num_ancestor_id_matches=ancestor_id_matches.size();

                //So im not sure how this works but something like this
                if(num_id_matches > 1){
                    std::cout<<"Well hot saussage.. more than 1 id match "<<num_id_matches<<std::endl;
                }else if(num_id_matches == 1){
                    //We have a direct match?
                    mcParticleToMCObjectMap[particle] = mcObjectVector[id_matches.front()];
                }else if(num_mother_id_matches == 1){
                    //We have a mother match? I guess this is like "Photon?->e+e-"
                    //mcParticleToMCObjectMap[particle] = mcObjectVector[mother_id_matches.front()];
                }else if(num_ancestor_id_matches == 1){
                    //We have a mother match? I guess this is like Neutron->photon->e+e-"
                    //mcParticleToMCObjectMap[particle] = mcObjectVector[ancestor_id_matches.front()];
                }else if(num_ancestor_id_matches >1 || num_mother_id_matches >1){
                    std::cout<<"Well hot saussage.. more than 1 mother or ancestor. Actually thats very reasonable hehe."<<num_mother_id_matches<<" "<<num_ancestor_id_matches<<std::endl;
                }else if(num_id_matches == 0 && num_ancestor_id_matches == 0 && num_mother_id_matches ==0){
                    std::cout<<"NO matches for trackid, mother trackid or ancestor trackid. Hmm"<<num_mother_id_matches<<" "<<num_ancestor_id_matches<<std::endl;
                }

                //What if multiple mothers matches?! no idea.


            }//MCParticleLoop

            return;
        }


int    SinglePhoton::photoNuclearTesting(std::vector<art::Ptr<simb::MCParticle>>& mcParticleVector){


    for(auto &mcp: mcParticleVector){
            int pdg = mcp->PdgCode();
            std::string end_process  = mcp->EndProcess();
            int status =  mcp->StatusCode()         ; 


            if(pdg==22){
                std::cout<<"PHOTO: "<<status<<" "<<end_process<<std::endl;
            }
    }


    return 0;

}

    /*
       void testbed( std::vector<art::Ptr<simb::MCParticle>>& mcParticleVector, const art::Event &evt){

       std::map<int,art::Ptr<simb::MCParticle> > crap_map;
       for(size_t j=0;j< mcParticleVector.size();j++){
       const art::Ptr<simb::MCParticle> mcp = mcParticleVector[j];
//std::cout<<"PARG: "<<j<<" trackid: "<<mcp->TrackId()<<" key: "<<mcp.key()<<std::endl;
crap_map[mcp->TrackId()] = mcParticleVector[mcp.key()];
}
art::ValidHandle<std::vector<simb::MCParticle>> const & mcParticleHandle= evt.getValidHandle<std::vector<simb::MCParticle>>("largeant");
art::FindManyP<simb::MCTruth,sim::GeneratedParticleInfo> genieMCTruthHandle(mcParticleHandle, evt, "largeant");

std::vector<art::Ptr<simb::MCTruth>> GenieMCTruth;
std::vector<sim::GeneratedParticleInfo const *> geninfo;



for(size_t i=0; i< mcParticleVector.size();i++){


art::Ptr<simb::MCParticle> nth_mother = mcParticleVector[i];

//if(nth_mother->PdgCode() != 22 && nth_mother->PdgCode() != 11) continue;

std::cout<<"----------------------------------------------------------"<<std::endl;
std::cout<<"SinglePhoton::testbed()\t||\t On Particle (trackid: "<<nth_mother->TrackId()<<") pdg: "<<nth_mother->PdgCode()<<", status_code "<<nth_mother->StatusCode()<<"  MotherTrackID: "<<nth_mother->Mother()<<std::endl;

int n_generation = 1;

while(nth_mother->Mother() != 0){

nth_mother = crap_map[nth_mother->Mother()]; 
std::cout<<"SinglePhoton::testbed()\t||\t -- and "<<n_generation<<"-mother trackid "<<nth_mother->TrackId()<<" is a pdg: "<<nth_mother->PdgCode()<<" and status_code "<<nth_mother->StatusCode()<<std::endl;
n_generation++;
}



GenieMCTruth.clear(); geninfo.clear(); //tidy up this loop
genieMCTruthHandle.get(mcParticleVector[i].key(),GenieMCTruth,geninfo);

std::cout<<"SinglePhoton::testbed()\t||\t "<<" GenieMCTruth.size() "<<GenieMCTruth.size()<<" geninfo.size() "<<geninfo.size()<<std::endl;
for(size_t k=0; k< GenieMCTruth.size(); k++){
std::cout<<"SinglePhoton::testbed()\t||\t -- "<<k<<": has "<<GenieMCTruth[k]->NParticles()<<" particlesand geninfo_index: "<<geninfo[k]->generatedParticleIndex()<<std::endl;
if((int)geninfo[k]->generatedParticleIndex() > GenieMCTruth[k]->NParticles() || geninfo[k]->generatedParticleIndex()==ULONG_MAX){
std::cout<<"SinglePhoton::testbed()\t||\t -- Thats way wrong.."<<std::endl;
}else{
const simb::MCParticle mp = GenieMCTruth[k]->GetParticle(geninfo[k]->generatedParticleIndex());
std::cout<<"SinglePhoton::testbed()\t||\t -- is a pdg: "<<mp.PdgCode()<<"  with statuscode:"<<mp.StatusCode()<<std::endl;
}
//std::cout<<"SinglePhoton::testbed()\t||\t "<<" "<<GenieMCTruth[0]->NParticles()<<" "<<geninfo[0]->generatedParticleIndex()<<std::endl;
}


}//particleloop
}

*/
}//namespace end
