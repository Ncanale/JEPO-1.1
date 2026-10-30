////////////////////////////////////////////////////////////////////////////////
//   SiTHOptRunAct.cc for the SiTH optical light-collection study             //
////////////////////////////////////////////////////////////////////////////////

#include "SiTHOptRunAct.hh"

#include "G4AccumulableManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4RootAnalysisManager.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"

#include <cmath>

namespace
{
	G4double Wavelength(G4double energy) { return h_Planck * c_light / energy / nm; }
}

SiTHOptRunAct::SiTHOptRunAct(const SiTHOptConfig& cfg)
	: m_Cfg(cfg)
{
	G4AccumulableManager::Instance()->Register(m_NGen);
	G4AccumulableManager::Instance()->Register(m_NDet);

	auto am = G4RootAnalysisManager::Instance();
	am->SetVerboseLevel(0);
	const G4double L = cfg.length / mm;
	const G4int nz = std::max(1, G4int(std::lround(L / 10.)));
	for ( const char* what : { "gen", "det" } )
		am->CreateH1(G4String(what) + "_z", "distance from end A (mm)", nz, 0., L);
	for ( const char* what : { "gen", "det" } )
		am->CreateH1(G4String(what) + "_lambda", "wavelength (nm)", 150, 350., 650.);
	am->CreateH1("det_time", "arrival time (ns)", 200, 0., 20.);
	for ( const char* what : { "gen", "det" } )
		am->CreateH2(G4String(what) + "_uv", "u = 2x/base vs v = y/height", 40, -1., 1., 20, 0., 1.);
	for ( const char* what : { "gen", "det" } )
		am->CreateH2(G4String(what) + "_uz", "u = 2x/base vs z (mm)", 40, -1., 1., std::max(1, nz / 2), 0., L);
}

void SiTHOptRunAct::BeginOfRunAction(const G4Run*)
{
	G4AccumulableManager::Instance()->Reset();
	G4RootAnalysisManager::Instance()->OpenFile(m_Cfg.out);
}

void SiTHOptRunAct::EndOfRunAction(const G4Run* run)
{
	G4AccumulableManager::Instance()->Merge();
	auto am = G4RootAnalysisManager::Instance();
	am->Write();
	am->CloseFile();

	if ( !IsMaster() || run->GetNumberOfEvent() == 0 ) return;
	const G4double nGen = m_NGen.GetValue(), nDet = m_NDet.GetValue();
	G4cout << "sithopt: generated " << nGen << " photons, detected " << nDet << ", collection efficiency "
	       << nDet / nGen << " +- " << std::sqrt(nDet) / nGen << G4endl;
}

void SiTHOptRunAct::AddGenerated(const G4ThreeVector& pos, G4int nPhot)
{
	m_NGen += nPhot;
	auto am = G4RootAnalysisManager::Instance();
	const G4double z = (pos.z() + m_Cfg.length / 2) / mm;
	const G4double u = 2 * pos.x() / m_Cfg.base, v = pos.y() / m_Cfg.height + 0.5;
	am->FillH1(0, z, nPhot);
	am->FillH2(0, u, v, nPhot);
	am->FillH2(2, u, z, nPhot);
}

void SiTHOptRunAct::AddWavelength(G4double energy)
{
	G4RootAnalysisManager::Instance()->FillH1(2, Wavelength(energy));
}

void SiTHOptRunAct::AddDetected(const G4ThreeVector& origin, G4double energy, G4double time)
{
	m_NDet += 1;
	auto am = G4RootAnalysisManager::Instance();
	const G4double z = (origin.z() + m_Cfg.length / 2) / mm;
	const G4double u = 2 * origin.x() / m_Cfg.base, v = origin.y() / m_Cfg.height + 0.5;
	am->FillH1(1, z);
	am->FillH1(3, Wavelength(energy));
	am->FillH1(4, time / ns);
	am->FillH2(1, u, v);
	am->FillH2(3, u, z);
}
