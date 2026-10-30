////////////////////////////////////////////////////////////////////////////////
//   SiTHOptRunAct.hh for the SiTH optical light-collection study             //
//                                                                            //
//   Histograms of generated and detected photons (merged over threads):      //
//     H1 0/1  z (distance from end A)          H2 0/1  u = 2x/base, v = y/h  //
//     H1 2/3  wavelength (nm)                  H2 2/3  u, z                  //
//     H1 4    arrival time of detected photons (ns, from emission)           //
////////////////////////////////////////////////////////////////////////////////

#ifndef SITHOPTRUNACT_H
#define SITHOPTRUNACT_H

#include "G4AccValue.hh"
#include "G4ThreeVector.hh"
#include "G4UserRunAction.hh"
#include "globals.hh"

#include "SiTHOptConfig.hh"

class G4Run;

class SiTHOptRunAct : public G4UserRunAction
{
  public:
	SiTHOptRunAct(const SiTHOptConfig& cfg);
	virtual ~SiTHOptRunAct() {}

	virtual void BeginOfRunAction(const G4Run* run);
	virtual void EndOfRunAction(const G4Run* run);

	void AddGenerated(const G4ThreeVector& pos, G4int nPhot);
	void AddWavelength(G4double energy);
	void AddDetected(const G4ThreeVector& origin, G4double energy, G4double time);

  private:
	const SiTHOptConfig& m_Cfg;
	G4AccValue<G4double> m_NGen{ 0. };
	G4AccValue<G4double> m_NDet{ 0. };
};

#endif
