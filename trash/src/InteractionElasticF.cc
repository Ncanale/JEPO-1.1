// File:	InteractionElasticF.cc
// Date:	13-Apr-2015 G.Macharashvili Dubna

#include "InteractionElasticF.hh"

#include <CLHEP/Random/RandFlat.h>
#include <G4ParticleTable.hh>
#include <G4RunManager.hh>

void*	InteractionElasticF::ptr = 0 ;

void*	InteractionElasticF::get() { return (ptr) ? ptr : ptr = new InteractionElasticF ; }

InteractionElasticF::~InteractionElasticF() {
        in.close() ;
	cout << "**** InteractionElasticF: read " << readEvents << " accepted " << acceptedEvents << " events ... " << endl ;
        }

InteractionElasticF::InteractionElasticF(const char *obj_name) : VInteractionGenerator(obj_name) {
	num_of_tracks = 1 ;
	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable() ;
// defaults
	particle = particleTable->FindParticle("deuteron") ;
	def[0] = particle ;
	kineticEnergy = 270.0 * CLHEP::MeV ;
	minTheta =  5.0 * CLHEP::deg ;
	maxTheta = 20.0 * CLHEP::deg ;
	readEvents = acceptedEvents = 0 ;
	}

void	InteractionElasticF::Init(G4ParticleDefinition *Xparticle,G4double XkineticEnergy,G4double t1,G4double t2) {
        if (XkineticEnergy != 0.0) kineticEnergy = XkineticEnergy ;
        if (t1 >= 0.0) minTheta = t1 ;
        if (t2 >= 0.0) {
                if (t2 > minTheta) maxTheta = t2 ;
                else maxTheta = minTheta + 0.001 ;
                }
	if (Xparticle != particle) {
		cerr << endl << "**** InteractionElasticF::Init() [F] particle definition is wrong ... " << endl << endl ;
		abort() ;
		}
// get data file ...
        G4String home = getenv("HOME") ;
        if (!home.length()) home = "." ;
	in.open("dC_Elastic.dat") ;
	if (!in.is_open()) {
	        in.open((home + "/dC_Elastic.dat").c_str()) ;
        	if (!in.is_open()) {
	        	in.open((home + "/edm-data/dC_Elastic.dat").c_str()) ;
                	if (!in.is_open()) {
                	        cerr << endl << "**** InteractionElasticF::Init() (F) ..." << endl ;;
                		cerr << "    ./dC_Elastic.dat " << endl ;
	                	cerr << "    ~/dC_Elastic.dat " << endl ;
	                	cerr << "    ~/edm-data/dC_Elastic.dat " << endl ;
        	        	cerr << "**** data file not exists. " << endl << endl ;
        	        	abort() ;
        	        	}
                        }
		}
        }

void	InteractionElasticF::GenerateInteraction() {
        SetPhi(0.0,0) ; // actually is defined by phiGenerator ... 20151101
        do {
		in >> ek[0] >> theta[0] ;
        	theta[0] += CLHEP::RandFlat::shoot(-0.1,+0.1) ;
		theta[0] *= CLHEP::deg ;
		readEvents++ ;
		} while (in.good() && (theta[0] < minTheta || theta[0] > maxTheta)) ;
	if (in.eof()) G4RunManager::GetRunManager()->AbortRun() ;
	else acceptedEvents++ ;
	}
	
// e-o-f
