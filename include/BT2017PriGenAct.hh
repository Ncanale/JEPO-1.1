#ifndef BT2017PRIGENACT_h
#define BT2017PRIGENACT_h 1

////////////////////////////////////////////////////////////////////////////////
//   BT2017PriGenAct.hh for beamtest2017 of JEDI                              //
//                                                                            //
//   This file is a header for BT2017PriGenAct class. You can set primary     //
// beam options in this class.                                                //
//                                                                            //
//                    - 21. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) //
////////////////////////////////////////////////////////////////////////////////

#include "globals.hh"
#include "Randomize.hh"

#include <CLHEP/Random/RandFlat.h>

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleDefinition.hh"
#include <G4RotationMatrix.hh>
#include <G4AffineTransform.hh>

#include "G4ParticleGun.hh"
#include "G4IonTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Event.hh"
#include "g4root.hh"

/*
#include <TROOT.h>
#include <TMath.h>
#include <TF1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TFile.h>
#include <TProfile.h>
#include <TVirtualFitter.h>
*/

#include "BT2017ConMan.hh"

//using namespace TMath;

class G4ParticleGun;
class G4Event;
class BT2017ConMan;

class BT2017PriGenAct: public G4VUserPrimaryGeneratorAction
{
  private:
    G4ParticleGun* PG;
    //Double_t theta;
    //Double_t phi;
    BT2017ConMan* m_CM;
/*
    TF1*   fGenTheta; // theta ran. gen. function
    TF1*   fGenPhi;   // phi ran. gen. function
    TH2D * hGenerator;
*/
    G4double 		m_WorldZ;

    G4double 		m_BeamPX;
    G4double 		m_BeamPY;
    G4double 		m_BeamPZ;

    G4double 		m_BeamDX;
    G4double 		m_BeamDY;

    G4double 		m_MinTheta;
    G4double 		m_MaxTheta;

    G4double		m_PolIndex;

    G4ThreeVector	m_MomDir;
    G4ThreeVector	m_Pol;
    G4ThreeVector	m_GunPos;

    G4String					m_parname;
    G4ParticleDefinition* 		m_Par;
    G4double 					m_KinEgy;
    G4int						m_eventID;

    G4RotationMatrix			*m_matrix ;
    G4double					m_beamAxisTheta , m_beamAxisPhi;
    

  public:
	BT2017PriGenAct(BT2017ConMan* CM);
	~BT2017PriGenAct();

  virtual void GeneratePrimaries(G4Event* anEvent);
	G4double GetTheta();
	G4double GetPhi();
	G4double GetKinEgy();
	G4double GetPosX();
	G4double GetPosY();
	G4int GetParticle();
	G4int GetGenEventID();
};

#endif
