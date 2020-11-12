////////////////////////////////////////////////////////////////////////////////
//   BT2017PriGenAct.cc for beamtest2017 of JEDI                              //
//                                                                            //
//   Definitions of BT2017PriGenAct class's member functions.                 //
//                                                                            //
//                    - 21. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) //
////////////////////////////////////////////////////////////////////////////////

#include "BT2017PriGenAct.hh"

//////////////////////////////////////////////////
//   Constructor and destructor                 //
//////////////////////////////////////////////////
BT2017PriGenAct::BT2017PriGenAct(BT2017ConMan *CM)
	: m_CM(CM)
{
	PG = new G4ParticleGun();
    
    //ROOT::EnableImplicitMT( m_CM->GetNofTRD() );
    
	// Gun position
	m_WorldZ = m_CM->GetWorldZ();
	m_BeamPX = m_CM->GetBeamAX();
	m_BeamPY = m_CM->GetBeamAY();
	m_BeamPZ = m_CM->GetBeamAZ();

    m_BeamDX = m_CM->GetBeamDX();
	m_BeamDY = m_CM->GetBeamDY();

	m_PolIndex = m_CM->GetBeamPO();

	m_MinTheta = m_CM->GetMinThe() * CLHEP::deg;
	m_MaxTheta = m_CM->GetMaxThe() * CLHEP::deg;
    
	// Set particle definition
	//m_parname = "deuteron";
    m_parname = m_CM->GetParName();
    //printf("-- set particle = %s\n", m_parname.c_str());
	G4ParticleTable *PT = G4ParticleTable::GetParticleTable();
	m_Par = PT->FindParticle(m_parname);
	PG->SetParticleDefinition(m_Par);
  //G4cerr<<"Charge: "<<PG->GetParticleCharge()<<"; Mass: "<<PG->GetParticleDefinition()->GetAtomicMass()<<G4endl;

	// Kinetic energy
	m_KinEgy = m_CM->GetBeamKE();
	PG->SetParticleEnergy(m_KinEgy);

	// Polarization
// 	m_Pol = G4ThreeVector(0., 1., 0.);
// 	PG->SetParticlePolarization(m_Pol);
    /*
    G4String fileName = getenv("HOME");
	fileName += "/simulation/pC_DSDO_152MeV.root";
    
    TFile* f = new  TFile( fileName );
	fGenTheta = (TF1*)f->Get("fpC_DSDO_152MeV")->Clone("fGenTheta");
	f->Close();
//    fGenTheta->Print();

    fileName = getenv("HOME");
    fileName += "/simulation/pC_Ay_152MeV.root";
    TFile* fAy = new  TFile( fileName );
	fGenTheta = (TF1*)fAy->Get("fpC_Ay_152MeV")->Clone("fAy");
	fAy->Close();
//    fGenTheta->Print();

    // read the generator histogram
    fileName = getenv("HOME");
    fileName += "/simulation/hPhiVsTheta_pol100.root";
    TFile *fin = TFile::Open(fileName);
    gROOT->cd(); //extremely neccessary 
    hGenerator = (TH2D*) fin->Get("hPhiVsTheta")->Clone("hGenerator");
    fin->Close();
//    hGenerator->Print();
    */
    
}

//////////////////////////////////////////////////
//  destructor                                  //
//////////////////////////////////////////////////
BT2017PriGenAct::~BT2017PriGenAct()
{
	delete PG;
}

//////////////////////////////////////////////////
//   Shoot!                                     //
//////////////////////////////////////////////////
void BT2017PriGenAct::GeneratePrimaries(G4Event *event)
{
	// event ID
	m_eventID = event->GetEventID();

    if( m_PolIndex ) {
      /*
        hGenerator->GetRandom2( phi, theta );
        m_beamAxisPhi   = phi   / 180. * M_PI;
        m_beamAxisTheta = theta / 180. * M_PI;
      */
    }        
    else {
        // Set theta distribution 
        m_beamAxisTheta = CLHEP::RandFlat::shoot(m_MinTheta, m_MaxTheta); // uniform distribution of polar angle
        m_beamAxisPhi   = CLHEP::RandFlat::shoot(0.0, 2.*M_PI );
        
//     if( m_MinTheta < 1. )
//         m_beamAxisTheta = CLHEP::RandFlat::shoot(m_MinTheta, m_MaxTheta); // uniform distribution of polar angle
//     else
//         m_beamAxisTheta = fGenTheta -> GetRandom() / 180. * M_PI;
    //printf("theta, phi [%3.1f , %3.1f]\n", m_beamAxisTheta, m_beamAxisPhi);
    
    
// 	// if a beam is not polarized the distribution of azimuth angle is uniform if not it is cosinusoidal
// 	if( m_PolIndex > 0) 
//         m_beamAxisPhi = CLHEP::RandFlat::shoot(-M_PI, M_PI);
// 	else
// 		m_beamAxisPhi = fGenPhi -> GetRandom();
    }
    
	// chosing the direction of momentum and rotating if
	m_MomDir.set( sin(m_beamAxisTheta) *cos(m_beamAxisPhi), // X
                  sin(m_beamAxisTheta) *sin(m_beamAxisPhi), // Y
                  cos(m_beamAxisTheta) );                   // Z
    
	m_matrix = new G4RotationMatrix;
	m_matrix->rotateY( -m_MomDir.x());
	m_matrix->rotateX(  m_MomDir.y());

  G4int NoE = 10000;
  if(m_eventID%(NoE/100)==0 && NoE>=100){G4cerr<<m_eventID/(NoE/100)<<"%"<<G4endl;}
	PG -> SetParticleMomentumDirection(m_MomDir);

	// 2D Gaussian
	G4double dX = G4RandGauss::shoot(0., m_BeamDX / mm / 3.0); // 3 sigma
	G4double dY = G4RandGauss::shoot(0., m_BeamDY / mm / 3.0); // 3 sigma
    
	m_GunPos = G4ThreeVector(m_BeamPX + dX * mm, m_BeamPY + dY * mm, m_BeamPZ*mm);
	PG->SetParticlePosition(m_GunPos);

	PG->GeneratePrimaryVertex(event);
}

//////////////////////////////////////////////////
//                                              //
//////////////////////////////////////////////////
// returns the polar angle of particle's momentum direction
G4double BT2017PriGenAct::GetTheta()
{
	return m_beamAxisTheta;
}

//////////////////////////////////////////////////
//                                              //
//////////////////////////////////////////////////
// returns the azimuth angle of particles momentum direction
G4double BT2017PriGenAct::GetPhi()
{
	return m_beamAxisPhi;
}

//////////////////////////////////////////////////
//                                              //
//////////////////////////////////////////////////
// returns the type of particle which is formed by generator in each event
G4int BT2017PriGenAct::GetParticle()
{
	G4int particleIndex = 0;
	if (m_parname == "deuteron") particleIndex = 45;
	else if (m_parname == "proton") particleIndex = 14;
	//	else if (m_parname == "neutron") particleIndex = 13;
	else if (m_parname == "alpha") particleIndex = 47;
	else if (m_parname == "muon") particleIndex = -13;
	return particleIndex;
}

//////////////////////////////////////////////////
//                                              //
//////////////////////////////////////////////////
// returns the initial kinetic energy of particle in each event
G4double BT2017PriGenAct::GetKinEgy()
{
	return m_KinEgy;
}

//////////////////////////////////////////////////
//                                              //
//////////////////////////////////////////////////
// returns the ID of an event
G4int BT2017PriGenAct::GetGenEventID()
{
	return m_eventID;
}
