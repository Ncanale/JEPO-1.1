////////////////////////////////////////////////////////////////////////////////
//   SiTHOptConfig.cc for the SiTH optical light-collection study             //
////////////////////////////////////////////////////////////////////////////////

#include "SiTHOptConfig.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace
{
	void Usage()
	{
		std::cout <<
			"sithopt [options]   lengths in mm, attenuation in cm\n"
			"  --base 60 --height 20 --length 420     triangular bar\n"
			"  --layout ONE|BOTH                       4 SiPMs on end A, or 2 + 2\n"
			"  --sipm 6                                SiPM side\n"
			"  --rwrap 0.95 --rmask 0.80               diffuse reflectances\n"
			"  --att 380 --sigma 0.01                  bulk attenuation, facet slope (rad)\n"
			"  --npad 1.43 --pad 1                     silicone pad index, thickness\n"
			"  --zpos -1                               distance from end A (<0: uniform)\n"
			"  --photons 50 --events 100000            photons per point, points\n"
			"  --threads 4 --seed 1 --out sithopt.root\n"
			"  --verbose 0                             /tracking/verbose (debug)\n";
	}
}

G4bool SiTHOptConfig::Parse(int argc, char** argv)
{
	for ( int i = 1; i < argc; i++ )
	{
		const char* key = argv[i];
		if ( !std::strcmp(key, "-h") || !std::strcmp(key, "--help") ) { Usage(); return false; }
		if ( i + 1 >= argc ) { std::cerr << "missing value for " << key << "\n"; return false; }
		const char* val = argv[++i];
		G4double x = std::atof(val);
		if      ( !std::strcmp(key, "--base") )    base = x * mm;
		else if ( !std::strcmp(key, "--height") )  height = x * mm;
		else if ( !std::strcmp(key, "--length") )  length = x * mm;
		else if ( !std::strcmp(key, "--layout") )  layout = val;
		else if ( !std::strcmp(key, "--sipm") )    sipm = x * mm;
		else if ( !std::strcmp(key, "--rwrap") )   rWrap = x;
		else if ( !std::strcmp(key, "--rmask") )   rMask = x;
		else if ( !std::strcmp(key, "--att") )     attBar = x * cm;
		else if ( !std::strcmp(key, "--sigma") )   sigmaAlpha = x;
		else if ( !std::strcmp(key, "--npad") )    nPad = x;
		else if ( !std::strcmp(key, "--pad") )     padThick = x * mm;
		else if ( !std::strcmp(key, "--zpos") )    zPos = x < 0 ? -1. : x * mm;
		else if ( !std::strcmp(key, "--photons") ) nPhot = std::atoi(val);
		else if ( !std::strcmp(key, "--events") )  nEvents = std::atoi(val);
		else if ( !std::strcmp(key, "--threads") ) threads = std::atoi(val);
		else if ( !std::strcmp(key, "--verbose") ) verbose = std::atoi(val);
		else if ( !std::strcmp(key, "--seed") )    seed = std::atol(val);
		else if ( !std::strcmp(key, "--out") )     out = val;
		else { std::cerr << "unknown option " << key << "\n"; Usage(); return false; }
	}
	if ( layout != "ONE" && layout != "BOTH" ) { std::cerr << "layout must be ONE or BOTH\n"; return false; }
	return true;
}

void SiTHOptConfig::Print() const
{
	G4cout << "sithopt: bar " << base / mm << " x " << height / mm << " x " << length / mm << " mm, layout "
	       << layout << ", SiPM " << sipm / mm << " mm\n"
	       << "         n_bar " << nBar << ", att " << attBar / cm << " cm, R_wrap " << rWrap
	       << ", sigma_alpha " << sigmaAlpha << ", R_mask " << rMask << ", n_pad " << nPad
	       << ", pad " << padThick / mm << " mm\n"
	       << "         " << nEvents << " points x " << nPhot << " photons, z "
	       << (zPos < 0 ? G4String("uniform") : std::to_string(zPos / mm) + " mm") << ", seed " << seed
	       << ", " << threads << " threads -> " << out << G4endl;
}
