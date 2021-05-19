////////////////////////////////////////////////////////////////////////////////
//   BT2017EveAct.cc for beamtest2017 of JEDI                                 //
//                                                                            //
//   Definitions of BT2017EveAct class's member functions. Details of user    //
// actions are here.                                                          //
//                                                                            //
//                    - 20. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) //
////////////////////////////////////////////////////////////////////////////////

#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

#include "BT2017EveAct.hh"
#include "BT2017Ana.hh"

//////////////////////////////////////////////////
//   Constructor                                //
//////////////////////////////////////////////////
BT2017EveAct::BT2017EveAct(BT2017ConMan* CM,BT2017PriGenAct* prigenact)
	: m_CM(CM),m_prigenact(prigenact)
{
	m_DetSet = m_CM -> GetDetSet();
}

//////////////////////////////////////////////////
//   Destructor                                 //
//////////////////////////////////////////////////
BT2017EveAct::~BT2017EveAct()
{
}

//////////////////////////////////////////////////
//   Begin of event action                      //
//////////////////////////////////////////////////
void BT2017EveAct::BeginOfEventAction(const G4Event* /*anEvent*/)
{
	// Initialize
	m_EDepCry.clear();
	m_EDepdE.clear();
	m_EDepTr.clear();
	for ( int i = 0; i < 120; i++ ) 	m_EDepCry[i]  = 0.0;
	for ( int i = 0; i <   4; i++ ) 	 m_EDepdE[i]  = 0.0;		
	for ( int i = 0; i <   28; i++ ) 	 m_EDepTr[i]  = 0.0;		
}

//////////////////////////////////////////////////
//   End of event action                        //
//////////////////////////////////////////////////
void BT2017EveAct::EndOfEventAction(const G4Event* anEvent)
{
  G4double NaN = std::numeric_limits<double>::quiet_NaN();
	//std::cout << "monacemebis Sevseba " << std::endl;
	// Get analysis manager
	G4AnalysisManager* AM = G4AnalysisManager::Instance();

	// Fill ntuple with result
	//G4int eID = m_prigenact -> GetGenEventID();
	G4int eID = anEvent -> GetEventID();
	AM -> FillNtupleIColumn(0, eID);

	G4int iDet = 1;

	// input parameters
	AM -> FillNtupleDColumn(iDet++, m_prigenact -> GetParticle());
	AM -> FillNtupleDColumn(iDet++, m_prigenact -> GetKinEgy());
	AM -> FillNtupleDColumn(iDet++, m_prigenact -> GetTheta());
	AM -> FillNtupleDColumn(iDet++, m_prigenact -> GetPhi());

	// filling initial parameters from generator
	//
	for ( int i = 0; i < 120; i++ )
		if ( m_DetSet[i] )
			AM -> FillNtupleDColumn(iDet++, m_EDepCry[i]);

	// dE filling
	for(int i = 0; i < 4; i++)
		AM -> FillNtupleDColumn(iDet++, m_EDepdE[i]);
	
	// tracker bar filling
  G4double Smear = 0.26;
	for(int i = 0; i < 28; i++)
		AM -> FillNtupleDColumn(iDet++, (m_EDepTr[i] <= 0.5 ? NaN : G4RandGauss::shoot(m_EDepTr[i],Smear*m_EDepTr[i]/2.355)));
	
	AM -> FillNtupleDColumn(iDet++, xcoor);
	
	AM -> AddNtupleRow();
}

//////////////////////////////////////////////////
//   Add energy deposition  to crystals         //
//////////////////////////////////////////////////
void BT2017EveAct::AddEDepCry(G4int id, G4double eD)
{
	m_EDepCry[id] += eD;
}

//////////////////////////////////////////////////
//   Add energy deposition  to scintillators    //
//////////////////////////////////////////////////
void BT2017EveAct::AddEDepdE(G4int id, G4double eD)
{
	m_EDepdE[id] += eD;
}

//////////////////////////////////////////////////
//   Add energy deposition  to trackers         //
//////////////////////////////////////////////////
void BT2017EveAct::AddEDepTr(G4int id, G4double eD)
{
	m_EDepTr[id] += eD;
}

void BT2017EveAct::AddXcoordinate(G4double Xcoor)
{
	xcoor = Xcoor;
}
