///////////////////////////////////////////////////////////////////////////////
//   beamtest2017.cc for beamtest2017 of JEDI                                 //
//                                                                            //
//   This is main function of the simulation program.                         //
//                                                                            //
//                    - 20. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) //
////////////////////////////////////////////////////////////////////////////////

//#undef G$MULTITHREADED

// TODO
// 1) correct option confligt -bg...

// c++
#include <iostream>

// c
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       
#include <getopt.h>

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "JePoDetCon.hh"
#include "BT2017PriGenAct.hh"
#include "BT2017ActIni.hh"
#include "BT2017ConMan.hh"

#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4String.hh"
#include "Randomize.hh"

#include "QGSP_BERT.hh"
#include "JediPhysicsListFactory.hh"
#include "G4HadronicParameters.hh"

// ROOT
//#include <TFile.h>

using namespace std;

void PrintHelp();

//////////////////////////////////////////////////
//   Main                                       //
//////////////////////////////////////////////////
int main (int argc, char** argv)
{
	int option, flag_b = 0, flag_g = 0, flag_m = 0, flag_bz = 0, flag_by = 0, flag_ty = 0;

	const char* optDic = "hbgm:c:z:y:t:"; // Option dictionary
	
	char * macro;
    const char * config = "config.cfg";
	
	G4double GunZPos = 0;
	G4double GunYPos = 0;
	G4double TarYPos = 0;

	// options to specify Y and Z position of beam and target
	static struct option long_options[] = {
                   {"BeamZpos",	required_argument, NULL, 'z' },
				   {"BeamYpos",	required_argument, NULL, 'y' },
				   {"TargYpos",	required_argument, NULL, 't' },
				   {0, 0, 0, 0}
               };

	//
	while ( (option = getopt_long(argc, argv, optDic, long_options, NULL)) != -1 ) // -1 means
    //parses all options.
	{
        switch ( option )
        {
            case 'h' :
		        PrintHelp();
				return 0;

			case 'b' :
                flag_b = 1;
                break;

			case 'g' :
                flag_g = 1;
                break;

			case 'm' :
                flag_m = 1;
                macro = optarg;
                break;

      case 'c' :
                config = optarg;
                break;

			case 'z' :
                flag_bz = 1;
                GunZPos = atof(optarg);
                break;
			case 'y' :
                flag_by = 1;
                GunYPos = atof(optarg);
                break;
			case 't' :
				flag_ty = 1;
				std::cout << "tarYpos flag " << flag_ty << std::endl;
				TarYPos = atof(optarg);
				break;
			default: /* '?' */
                fprintf(stderr, "Usage: %s [-h]\n",  argv[0]);
                   exit(EXIT_FAILURE);
        }              
    }

	// Randomizer
	CLHEP::RanluxEngine defaultEngine(1234567, 4);
	G4Random::setTheEngine(&defaultEngine);
	G4int seed = time(NULL);
	G4Random::setTheSeed(seed);

	// Detect interactive mode (if any arguments) and define UI session
	G4UIExecutive* UI = 0;

	// graphical UI
	if ( flag_g ) UI = new G4UIExecutive(argc, argv);

	// Get configuration from file
	BT2017ConMan* BT2017CM = new BT2017ConMan(config);

	// Change value of beam z position in needed
	if ( flag_by ) BT2017CM->SetBeamAY(GunYPos);
	if ( flag_bz ) BT2017CM->SetBeamAZ(GunZPos);
	if ( flag_ty ) BT2017CM->SetTarPoY(TarYPos+15);
	
	
	// Run manager
	G4MTRunManager* RM = new G4MTRunManager();
	G4int nofTRD = BT2017CM -> GetNofTRD();
	RM -> SetNumberOfThreads(nofTRD);

	// Detector construction from configuration (Geometry)
	// We define everything about geomtrical setup in this class.
	JePoDetCon* BT2017DC = new JePoDetCon(BT2017CM);
	RM -> SetUserInitialization(BT2017DC);

	// Physics list to be used
	if ( BT2017CM -> GetUseJED() )
	{
		G4VModularPhysicsList* PL = JediPhysicsListFactory::Create();
		RM -> SetUserInitialization(PL);
	}
	else
	{
		G4VModularPhysicsList* PL = new QGSP_BERT(0);
		PL -> SetVerboseLevel(0);
		RM -> SetUserInitialization(PL);
	}

	// User actions
	G4HadronicParameters::Instance()->SetVerboseLevel(0);
	RM -> SetUserInitialization( new BT2017ActIni(BT2017CM) );

	// Initialize
	RM -> Initialize();

	// Visualization manger
	G4VisManager* VM = new G4VisExecutive();
	VM -> Initialize();

	// Get the pointer to the user interface manager
	G4UImanager* UM = G4UImanager::GetUIpointer();

	// Process macro or start UI session
	G4String command = "/control/execute ";

	if ( !UI )
	{	
		// batch mode
		if ( flag_m ) UM -> ApplyCommand( command + macro );
    else if ( flag_b )
    {
      G4UIsession* US = new G4UIterminal(new G4UItcsh);
      US -> SessionStart();
      delete US;
    }
	}

	else
	{
		// Interactive mode
		G4String fileName = "vis.mac";
		UM -> ApplyCommand(command + fileName);
		UI -> SessionStart();

		//delete UI;
	}

	// Job termination
	// Free the store: user actions, physics_list and detector_description are
	// owned and deleted by the run manager, so they should not be deleted 
	// in the main() program.
	delete RM;
	delete VM;

	return 0;
}

//////////////////////////////////////////////////
//   HELP                                       //
//////////////////////////////////////////////////
void PrintHelp()
{
    cout << "usage: jepo [-b] [-m macrofile] [-c configfile]" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "  jepo -b -m myRun.mac -c myConfig.cfg  # Run in batch mode with macro and config." << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "  -b  Execute in batch mode" << endl;
    cout << "      Note: Default is command mode" << endl;
    cout << "  -g  Execute in graphical mode" << endl;
    cout << "  -h  Show help message" << endl;
    cout << "  -m  Run with macro" << endl;
    cout << "  -c  Use specified config" << endl;
    cout << "      Default is config.cfg" << endl;
	cout << "  --BeamZpos  Use specified z position for beam" << endl;
	cout << "  --BeamYpos  Use specified z position for beam" << endl;
	cout << "  --TargYpos  Use specified z position for beam" << endl;
    cout << endl;
}
