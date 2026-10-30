////////////////////////////////////////////////////////////////////////////////
//   SiTHOptDetCon.hh for the SiTH optical light-collection study             //
//                                                                            //
//   One EJ-200 triangular bar wrapped in Teflon (diffuse reflector with an   //
//   air gap; black Tedlar outside, so nothing comes back from outside), a    //
//   silicone pad on the read-out end face and, behind it, the SiPMs and the  //
//   white mask of the pre-amplifier board.                                   //
////////////////////////////////////////////////////////////////////////////////

#ifndef SITHOPTDETCON_H
#define SITHOPTDETCON_H

#include "G4VUserDetectorConstruction.hh"
#include "G4TwoVector.hh"
#include "globals.hh"

#include <vector>

#include "SiTHOptConfig.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4VSolid;

class SiTHOptDetCon : public G4VUserDetectorConstruction
{
  public:
	SiTHOptDetCon(const SiTHOptConfig& cfg);
	virtual ~SiTHOptDetCon() {}

	virtual G4VPhysicalVolume* Construct();

  private:
	struct Square { G4TwoVector centre; G4double phi; };

	// SiPMs along one slanted side (side -1 left, +1 right), centred on the
	// side midpoint with the given offsets along the side
	std::vector<Square> AlongSide(G4int side, const std::vector<G4double>& offsets) const;
	void PlaceReadout(G4int end, const std::vector<Square>& squares, G4LogicalVolume* worldLV);

	const SiTHOptConfig& m_Cfg;
	std::vector<G4TwoVector> m_Triangle;
	G4double m_ROThick;
	G4VPhysicalVolume* m_WorldPV;
};

#endif
