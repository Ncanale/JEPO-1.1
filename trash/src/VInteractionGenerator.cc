// File:	VInteractionGenerator.cc
// Date:	14-Apr-2015 G.Macharashvili Dubna

#include "VInteractionGenerator.hh"

VInteractionGenerator::VInteractionGenerator(const char *xname) : name(xname) {
	for (int k = 0 ; k < maximumTracks ; k++) phi[k] = 1000.0 ;
	beam = target = 0 ;
	beamMomentum = G4ThreeVector(0,0,0) ;
	num_of_tracks = 0 ;
	kineticEnergy = 270 * CLHEP::MeV ;
	minTheta = 0 ;
	maxTheta = 25.0 * CLHEP::deg ;
	}

VInteractionGenerator::~VInteractionGenerator() {}

// e-o-f
