////////////////////////////////////////////////////////////////////////////////
//   sithopt.cc: light collection of one SiTH triangular bar                  //
//                                                                            //
//   Optical photons are generated inside an EJ-200 bar and tracked to the    //
//   SiPMs: the collection efficiency (photons reaching the SiPMs over        //
//   photons emitted) and its dependence on the emission point give the      //
//   relative number of photoelectrons of different bar geometries.           //
//                                                                            //
//                       - 29. Oct. 2026. Nicola Canale (cnlncl@unife.it)     //
////////////////////////////////////////////////////////////////////////////////

#include "FTFP_BERT.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhysics.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "Randomize.hh"

#include "SiTHOptActIni.hh"
#include "SiTHOptConfig.hh"
#include "SiTHOptDetCon.hh"

int main(int argc, char** argv)
{
	SiTHOptConfig cfg;
	if ( !cfg.Parse(argc, argv) ) return 1;
	cfg.Print();

	G4Random::setTheSeed(cfg.seed);
	auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::MT);
	runManager->SetNumberOfThreads(cfg.threads);

	runManager->SetUserInitialization(new SiTHOptDetCon(cfg));
	auto physics = new FTFP_BERT(0);
	physics->RegisterPhysics(new G4OpticalPhysics());
	runManager->SetUserInitialization(physics);
	auto params = G4OpticalParameters::Instance();
	params->SetProcessActivation("Cerenkov", false);
	params->SetProcessActivation("Scintillation", false);
	params->SetProcessActivation("OpWLS", false);
	params->SetProcessActivation("OpWLS2", false);
	params->SetProcessActivation("OpRayleigh", false);
	params->SetProcessActivation("OpMieHG", false);

	runManager->SetUserInitialization(new SiTHOptActIni(cfg));
	runManager->Initialize();
	if ( cfg.verbose > 0 )
	{
		auto ui = G4UImanager::GetUIpointer();
		ui->ApplyCommand("/particle/select opticalphoton");
		ui->ApplyCommand("/particle/process/dump");
		ui->ApplyCommand("/tracking/verbose " + std::to_string(cfg.verbose));
	}
	runManager->BeamOn(cfg.nEvents);

	delete runManager;
	return 0;
}
