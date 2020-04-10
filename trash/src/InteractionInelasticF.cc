// File:	InteractionInelasticF.cc
// Date:	13-Apr-2015 G.Macharashvili Dubna

#include "InteractionInelasticF.hh"
#include <CLHEP/Random/RandFlat.h>
#include <G4ParticleTable.hh>
#include <G4RunManager.hh>

void*	InteractionInelasticF::ptr = 0 ;

void*	InteractionInelasticF::get() { return (ptr) ? ptr : ptr = new InteractionInelasticF ; }

InteractionInelasticF::~InteractionInelasticF() {
        in.close() ;
        }

InteractionInelasticF::InteractionInelasticF(const char *obj_name) : VInteractionGenerator(obj_name) {
	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable() ;
	d = particleTable->FindParticle("deuteron") ;
	p = particleTable->FindParticle("proton") ;
	n = particleTable->FindParticle("neutron") ;
	a = particleTable->FindParticle("alpha") ;

	kineticEnergy = 270 * CLHEP::MeV ;
	minTheta =  5.0 * CLHEP::deg ;
	maxTheta = 20.0 * CLHEP::deg ;
	}

void	InteractionInelasticF::Init(G4ParticleDefinition *Xparticle,G4double XkineticEnergy,G4double t1,G4double t2) {
        if (minTheta >= 0.0) minTheta = t1 ;
        if (maxTheta >= 0.0) {
                if (maxTheta > minTheta) maxTheta = t2 ;
                else maxTheta = minTheta + t2 ;
                }
// get data file ...
        G4String home = getenv("HOME") ;
	in.open("dC_Inelastic.dat") ;
	if (!in.is_open()) {
	        in.open((home + "/dC_Inelastic.dat").c_str()) ;
        	if (!in.is_open()) {
	        	in.open((home + "/edm-data/dC_Inelastic.dat").c_str()) ;
                	if (!in.is_open()) {
                	        cerr << endl << "**** InteractionInelasticF::Init() (F) ..." << endl ;;
                		cerr << "    ./dC_Inelastic.dat " << endl ;
	                	cerr << "    ~/dC_Inelastic.dat " << endl ;
	                	cerr << "    ~/edm-data/dC_Inelastic.dat " << endl ;
        	        	cerr << "**** data file not exists. " << endl << endl ;
        	        	abort() ;
        	        	}
                        }
		}
        }

void	InteractionInelasticF::GenerateInteraction() {
        G4int trk = 0 ;
	char c ;
	G4int Ntotal , Nrecorded ;
	if (in.eof()) G4RunManager::GetRunManager()->AbortRun() ;
        in >> c >> Ntotal >> Nrecorded ;
        for (G4int t = 0 ; t < Nrecorded ; t++) {
		in >> c >> ek[trk] >> theta[trk] >> phi[trk] ;
		if (in.eof()) G4RunManager::GetRunManager()->AbortRun() ;
		theta[trk] += CLHEP::RandFlat::shoot(-0.2,+0.2) ;
		theta[trk] *= CLHEP::deg ;
		if (theta[trk] < minTheta || theta[trk] > maxTheta) continue ; // theta limits applied ...
		phi[trk] *= CLHEP::deg ;
		def[trk] = d ;
		if (c == 'p') def[trk] = p ;
		else if (c == 'n') def[trk] = n ;
		else if (c == 'a') def[trk] = a ;
//		if (def[trk] == d) continue ; // remove deuterons ...
//		if (def[trk] == p) continue ; // remove protons ...
//		if (def[trk] == n) continue ; // remove neutrons ...
//		if (def[trk] == a) continue ; // remove alphas ...
		trk++ ;
		}
        num_of_tracks = trk ;
	}
	
// e-o-f
