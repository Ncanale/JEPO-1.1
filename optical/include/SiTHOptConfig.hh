////////////////////////////////////////////////////////////////////////////////
//   SiTHOptConfig.hh for the SiTH optical light-collection study             //
//                                                                            //
//   Parameters of one run, from the command line (sithopt --help).           //
//   Read-only after parsing: shared by all threads.                          //
////////////////////////////////////////////////////////////////////////////////

#ifndef SITHOPTCONFIG_H
#define SITHOPTCONFIG_H

#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"

struct SiTHOptConfig
{
	// Bar: isosceles triangle (base, height) extruded along z (length)
	G4double base   = 60. * mm;
	G4double height = 20. * mm;
	G4double length = 420. * mm;

	// Read-out: 4 SiPMs of side sipm. ONE = all four on end A, two along each
	// slanted side (2018 pre-amplifier board); BOTH = one per slanted side at
	// each end
	G4String layout  = "ONE";
	G4double sipm    = 6. * mm;
	G4double sipmGap = 0.2 * mm;   // between the two SiPMs of a side
	G4double inset   = 0.5 * mm;   // SiPM edge to slanted side of the face

	// Optics
	G4double nBar     = 1.58;          // EJ-200
	G4double attBar   = 380. * cm;     // EJ-200 bulk attenuation length
	G4double rWrap    = 0.95;          // Teflon, diffuse, air gap
	G4double sigmaAlpha = 0.01;        // rad, bar surface micro-facets
	G4double rMask    = 0.80;          // white 3D-printed mask around the SiPMs
	G4double nPad     = 1.43;          // silicone optical pad
	G4double padThick = 1. * mm;
	G4double nSiPM    = 1.55;          // SiPM entrance window

	// Generation: points uniform in the bar volume (or at a fixed distance
	// zPos from end A when zPos >= 0), nPhot isotropic photons per point
	G4double zPos    = -1.;
	G4int    nPhot   = 50;
	G4int    nEvents = 100000;

	G4int    verbose = 0;              // /tracking/verbose
	G4int    threads = 4;
	long     seed    = 1;
	G4String out     = "sithopt.root";

	G4bool Parse(int argc, char** argv);
	void   Print() const;
};

#endif
