////////////////////////////////////////////////////////////////////////////////
//   SiTHOptPriGen.hh for the SiTH optical light-collection study             //
//                                                                            //
//   Each event: one point uniform in the bar cross-section (uniform in z or  //
//   at a fixed z), nPhot optical photons, isotropic, randomly polarised,     //
//   with the EJ-200 emission spectrum.                                       //
////////////////////////////////////////////////////////////////////////////////

#ifndef SITHOPTPRIGEN_H
#define SITHOPTPRIGEN_H

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

#include <vector>

#include "SiTHOptConfig.hh"

class G4Event;
class SiTHOptRunAct;

class SiTHOptPriGen : public G4VUserPrimaryGeneratorAction
{
  public:
	SiTHOptPriGen(const SiTHOptConfig& cfg, SiTHOptRunAct* runAct);
	virtual ~SiTHOptPriGen() {}

	virtual void GeneratePrimaries(G4Event* event);

  private:
	G4double SampleEnergy() const;

	const SiTHOptConfig& m_Cfg;
	SiTHOptRunAct* m_RunAct;
	std::vector<G4double> m_Lambda, m_Cdf;
};

#endif
