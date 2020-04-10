#ifndef BT2017DETMAP_h
#define BT2017DETMAP_h 1

////////////////////////////////////////////////////////////////////////////////
//   BT2017DetMap.hh for beamtest2017 of JEDI                                 //
//                                                                            //
//   This file is a header for BT2017DetMap class. I made this class to get   //
// mapping between detector ID and detector name easily.                      //
//                                                                            //
//                    - 22. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) //
////////////////////////////////////////////////////////////////////////////////

#include <map>
#include "G4String.hh"

class BT2017DetMap
{
  public:
	// Constructor and destructor
	BT2017DetMap();
	~BT2017DetMap();

	// Initializaion
	void Init();

	// Get
	G4String GetDetNameFromDetID(G4int id);
	G4int GetDetIDFromDetName(G4String name);
	G4String GetdENameFromdEID(G4int id);
	G4int GetdEIDFromdEName(G4String name);
	G4String GetTrNameFromTrID(G4int id);
	G4int GetTrIDFromTrName(G4String name);

  private:
	// naming for LYSO
	std::map<G4int, G4String> m_DetNameFromDetID;
	std::map<G4String, G4int> m_DetIDFromDetName;

	// naming for pl. dE Scintillators
	std::map<G4int, G4String> m_dENameFromdEID;
	std::map<G4String, G4int> m_dEIDFromdEName;

	// naming for Trackers
	std::map<G4int, G4String> m_TrNameFromTrID;
	std::map<G4String, G4int> m_TrIDFromTrName;
};

#endif
