////////////////////////////////////////////////////////////////////////////////
//   SiTHOptPriGen.cc for the SiTH optical light-collection study             //
////////////////////////////////////////////////////////////////////////////////

#include "SiTHOptPriGen.hh"

#include "G4Event.hh"
#include "G4OpticalPhoton.hh"
#include "G4PhysicalConstants.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <algorithm>
#include <cmath>

#include "SiTHOptRunAct.hh"

SiTHOptPriGen::SiTHOptPriGen(const SiTHOptConfig& cfg, SiTHOptRunAct* runAct)
	: m_Cfg(cfg), m_RunAct(runAct)
{
	// EJ-200 emission spectrum (relative intensity), read off the Eljen data
	// sheet: peak at 425 nm. Only used to weight the detected photons with a
	// SiPM PDE afterwards: the optical constants are wavelength independent.
	const std::vector<std::pair<G4double, G4double>> spectrum = {
		{ 395, 0.00 }, { 400, 0.05 }, { 405, 0.18 }, { 410, 0.42 }, { 415, 0.70 }, { 420, 0.90 },
		{ 425, 1.00 }, { 430, 0.93 }, { 435, 0.82 }, { 440, 0.72 }, { 445, 0.62 }, { 450, 0.53 },
		{ 455, 0.45 }, { 460, 0.38 }, { 465, 0.31 }, { 470, 0.25 }, { 475, 0.20 }, { 480, 0.16 },
		{ 490, 0.10 }, { 500, 0.06 }, { 510, 0.035 }, { 520, 0.02 }, { 540, 0.005 }, { 560, 0.00 } };
	G4double sum = 0.;
	m_Lambda.push_back(spectrum.front().first);
	m_Cdf.push_back(0.);
	for ( size_t i = 1; i < spectrum.size(); i++ )
	{
		sum += 0.5 * (spectrum[i].second + spectrum[i - 1].second) * (spectrum[i].first - spectrum[i - 1].first);
		m_Lambda.push_back(spectrum[i].first);
		m_Cdf.push_back(sum);
	}
	for ( auto& c : m_Cdf ) c /= sum;
}

G4double SiTHOptPriGen::SampleEnergy() const
{
	const G4double r = G4UniformRand();
	const size_t i = std::upper_bound(m_Cdf.begin(), m_Cdf.end(), r) - m_Cdf.begin();
	const size_t k = std::min(std::max<size_t>(i, 1), m_Cdf.size() - 1);
	const G4double f = (r - m_Cdf[k - 1]) / (m_Cdf[k] - m_Cdf[k - 1]);
	const G4double lambda = (m_Lambda[k - 1] + f * (m_Lambda[k] - m_Lambda[k - 1])) * nm;
	return h_Planck * c_light / lambda;
}

void SiTHOptPriGen::GeneratePrimaries(G4Event* event)
{
	// Point uniform in the triangle (-b/2,-h/2), (b/2,-h/2), (0,h/2)
	G4double r1 = G4UniformRand(), r2 = G4UniformRand();
	if ( r1 + r2 > 1. ) { r1 = 1. - r1; r2 = 1. - r2; }
	const G4double b = m_Cfg.base, h = m_Cfg.height, L = m_Cfg.length;
	const G4double x = -b / 2 + r1 * b + r2 * b / 2;
	const G4double y = -h / 2 + r2 * h;
	const G4double z = m_Cfg.zPos < 0 ? (G4UniformRand() - 0.5) * L : -L / 2 + m_Cfg.zPos;
	const G4ThreeVector pos(x, y, z);

	auto vertex = new G4PrimaryVertex(pos, 0.);
	for ( G4int i = 0; i < m_Cfg.nPhot; i++ )
	{
		const G4double cost = 2 * G4UniformRand() - 1, sint = std::sqrt(1 - cost * cost);
		const G4double phi = twopi * G4UniformRand();
		const G4ThreeVector dir(sint * std::cos(phi), sint * std::sin(phi), cost);
		const G4ThreeVector e1 = dir.orthogonal().unit(), e2 = dir.cross(e1);
		const G4double psi = twopi * G4UniformRand();

		const G4double energy = SampleEnergy();
		auto photon = new G4PrimaryParticle(G4OpticalPhoton::Definition());
		photon->SetKineticEnergy(energy);
		photon->SetMomentumDirection(dir);
		photon->SetPolarization(std::cos(psi) * e1 + std::sin(psi) * e2);
		vertex->SetPrimary(photon);
		m_RunAct->AddWavelength(energy);
	}
	event->AddPrimaryVertex(vertex);
	m_RunAct->AddGenerated(pos, m_Cfg.nPhot);
}
