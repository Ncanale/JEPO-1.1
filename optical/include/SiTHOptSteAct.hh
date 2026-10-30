////////////////////////////////////////////////////////////////////////////////
//   SiTHOptSteAct.hh for the SiTH optical light-collection study             //
//                                                                            //
//   A photon transmitted into a SiPM is detected (PDE applied in the         //
//   analysis); a photon transmitted into the world (pad edges) is lost.      //
////////////////////////////////////////////////////////////////////////////////

#ifndef SITHOPTSTEACT_H
#define SITHOPTSTEACT_H

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class G4LogicalVolume;
class G4Step;
class SiTHOptRunAct;

class SiTHOptSteAct : public G4UserSteppingAction
{
  public:
	SiTHOptSteAct(SiTHOptRunAct* runAct);
	virtual ~SiTHOptSteAct() {}

	virtual void UserSteppingAction(const G4Step* step);

  private:
	SiTHOptRunAct* m_RunAct;
	G4LogicalVolume* m_World = nullptr;
	G4LogicalVolume* m_SiPM[2] = { nullptr, nullptr };
};

#endif
