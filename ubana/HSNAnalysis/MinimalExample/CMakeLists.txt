# basic source code CMakeLists.txt

art_make(BASENAME_ONLY
	LIB_LIBRARIES
		lardataobj_RecoBase
		lardataobj_AnalysisBase
		larcorealg_Geometry
    larcore_Geometry_Geometry_service
		lardata_Utilities
		nusimdata::SimulationBase
		messagefacility::MF_MessageLogger
		
		fhiclcpp::fhiclcpp
		cetlib::cetlib
                cetlib_except::cetlib_except
		CLHEP::CLHEP
	MODULE_LIBRARIES
		larreco_RecoAlg
		larreco_RecoAlg_Cluster3DAlgs
		larsim_Simulation
		lardataobj_RawData
		lardata_RecoObjects
		lardataobj_RecoBase
		lardataobj_MCBase
		lardataobj_AnalysisBase
		larcorealg_Geometry
    larcore_Geometry_Geometry_service
		lardata_Utilities
		nusimdata::SimulationBase
		art::Framework_Core
		art::Framework_Principal
		art::Framework_Services_Registry
		art_root_io::tfile_support ROOT::Core
		art_root_io::TFileService_service
		art::Framework_Services_Optional_RandomNumberGenerator_service
		art::Persistency_Common
		art::Persistency_Provenance
		art::Utilities
		messagefacility::MF_MessageLogger
		
		fhiclcpp::fhiclcpp
		cetlib::cetlib
                cetlib_except::cetlib_except
		CLHEP::CLHEP
	) ### MIGRATE-ACTION-RECOMMENDED (migrate-3.21.01) - deprecated: use art_make_library(), art_dictonary(), and cet_build_plugin() with explicit source lists and plugin base types

install_headers()
install_source()
install_fhicl()
