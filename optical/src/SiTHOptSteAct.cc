////////////////////////////////////////////////////////////////////////////////
//   SiTHOptSteAct.cc for the SiTH optical light-collection study             //
////////////////////////////////////////////////////////////////////////////////

#include "SiTHOptSteAct.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"
#include "G4Track.hh"

#include "SiTHOptRunAct.hh"

SiTHOptSteAct::SiTHOptSteAct(SiTHOptRunAct* runAct)
	: m_RunAct(runAct)
{
}

void SiTHOptSteAct::UserSteppingAction(const G4Step* step)
{
	G4Track* track = step->GetTrack();
	if ( track->GetDefinition() != G4OpticalPhoton::Definition() ) return;

	if ( !m_World )
	{
		auto store = G4LogicalVolumeStore::GetInstance();
		m_World = store->GetVolume("World");
		m_SiPM[0] = store->GetVolume("SiPM_A", false);
		m_SiPM[1] = store->GetVolume("SiPM_B", false);
	}

	// The volume the photon travels in. On a boundary step the post-step point
	// already belongs to the next volume, also when the photon is reflected;
	// a reflected photon then makes a zero-length step back from there
	if ( step->GetStepLength() <= 0. ) return;
	const auto pre = step->GetPreStepPoint();
	const G4LogicalVolume* lv = pre->GetPhysicalVolume()->GetLogicalVolume();
	if ( lv == m_SiPM[0] || (m_SiPM[1] && lv == m_SiPM[1]) )
	{
		m_RunAct->AddDetected(track->GetVertexPosition(), track->GetTotalEnergy(), pre->GetLocalTime());
		track->SetTrackStatus(fStopAndKill);
	}
	else if ( lv == m_World )
		track->SetTrackStatus(fStopAndKill);
}
