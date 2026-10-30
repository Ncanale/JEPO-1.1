////////////////////////////////////////////////////////////////////////////////
//   SiTHOptDetCon.cc for the SiTH optical light-collection study             //
////////////////////////////////////////////////////////////////////////////////

#include "SiTHOptDetCon.hh"

#include "G4Box.hh"
#include "G4DisplacedSolid.hh"
#include "G4ExtrudedSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4NistManager.hh"
#include "G4OpticalSurface.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"

#include <cmath>

namespace
{
	// Optical constants are flat over the EJ-200 emission (345-690 nm)
	const std::vector<G4double> kEnergy = { 1.8 * eV, 3.6 * eV };

	void AddFlat(G4MaterialPropertiesTable* mpt, const char* key, G4double value)
	{
		mpt->AddProperty(key, kEnergy, { value, value });
	}

	G4Material* OpticalMaterial(const G4String& name, const G4String& base, G4double rindex, G4double absLength)
	{
		auto nist = G4NistManager::Instance();
		auto ref = nist->FindOrBuildMaterial(base);
		auto mat = nist->BuildMaterialWithNewDensity(name, base, ref->GetDensity());
		auto mpt = new G4MaterialPropertiesTable();
		AddFlat(mpt, "RINDEX", rindex);
		if ( absLength > 0 ) AddFlat(mpt, "ABSLENGTH", absLength);
		mat->SetMaterialPropertiesTable(mpt);
		return mat;
	}
}

SiTHOptDetCon::SiTHOptDetCon(const SiTHOptConfig& cfg)
	: m_Cfg(cfg), m_ROThick(0.5 * mm), m_WorldPV(nullptr)
{
	const G4double b = cfg.base, h = cfg.height;
	m_Triangle = { { -b / 2, -h / 2 }, { 0., h / 2 }, { b / 2, -h / 2 } };   // clockwise
}

std::vector<SiTHOptDetCon::Square> SiTHOptDetCon::AlongSide(G4int side, const std::vector<G4double>& offsets) const
{
	const G4TwoVector a = side < 0 ? m_Triangle[0] : m_Triangle[2];
	const G4TwoVector c = m_Triangle[1];
	const G4TwoVector t = (c - a).unit();
	const G4TwoVector n = side < 0 ? G4TwoVector(t.y(), -t.x()) : G4TwoVector(-t.y(), t.x());   // inwards
	const G4TwoVector mid = 0.5 * (a + c);

	std::vector<Square> squares;
	for ( G4double off : offsets )
		squares.push_back({ mid + off * t + (m_Cfg.inset + m_Cfg.sipm / 2) * n, std::atan2(t.y(), t.x()) });
	return squares;
}

void SiTHOptDetCon::PlaceReadout(G4int end, const std::vector<Square>& squares, G4LogicalVolume* worldLV)
{
	const G4String tag = end < 0 ? "_A" : "_B";
	const G4double zFace = end * m_Cfg.length / 2;

	// Silicone pad over the whole end face
	auto padMat = OpticalMaterial("Pad" + tag, "G4_POLYETHYLENE", m_Cfg.nPad, 1. * m);
	auto padSV = new G4ExtrudedSolid("Pad" + tag, m_Triangle, m_Cfg.padThick / 2);
	auto padLV = new G4LogicalVolume(padSV, padMat, "Pad" + tag);
	auto padPV = new G4PVPlacement(nullptr, G4ThreeVector(0., 0., zFace + end * m_Cfg.padThick / 2), padLV,
	                               "Pad" + tag, worldLV, false, 0, true);

	// Board plane behind the pad: SiPMs (clipped to the face) and white mask
	auto plane = new G4ExtrudedSolid("Plane" + tag, m_Triangle, m_ROThick / 2);
	auto square = new G4Box("Square" + tag, m_Cfg.sipm / 2, m_Cfg.sipm / 2, m_ROThick / 2 + 0.1 * mm);
	G4VSolid* sensors = nullptr;
	for ( const auto& sq : squares )
	{
		G4Transform3D tr(G4RotationMatrix().rotateZ(sq.phi), G4ThreeVector(sq.centre.x(), sq.centre.y(), 0.));
		sensors = sensors ? static_cast<G4VSolid*>(new G4UnionSolid("Sensors" + tag, sensors, square, tr))
		                  : static_cast<G4VSolid*>(new G4DisplacedSolid("Sensors" + tag, square, tr));
	}
	auto sipmSV = new G4IntersectionSolid("SiPM" + tag, plane, sensors);
	auto maskSV = new G4SubtractionSolid("Mask" + tag, plane, sensors);

	auto sipmMat = OpticalMaterial("SiPMWindow" + tag, "G4_Si", m_Cfg.nSiPM, 1. * um);
	auto maskMat = OpticalMaterial("MaskPlastic" + tag, "G4_POLYSTYRENE", 1.5, 1. * um);
	auto sipmLV = new G4LogicalVolume(sipmSV, sipmMat, "SiPM" + tag);
	auto maskLV = new G4LogicalVolume(maskSV, maskMat, "Mask" + tag);
	const G4ThreeVector pos(0., 0., zFace + end * (m_Cfg.padThick + m_ROThick / 2));
	new G4PVPlacement(nullptr, pos, sipmLV, "SiPM" + tag, worldLV, false, 0, true);
	auto maskPV = new G4PVPlacement(nullptr, pos, maskLV, "Mask" + tag, worldLV, false, 0, true);

	// White mask: diffuse reflector in contact with the pad
	auto mask = new G4OpticalSurface("MaskSurface" + tag, unified, groundfrontpainted, dielectric_dielectric);
	auto mpt = new G4MaterialPropertiesTable();
	AddFlat(mpt, "REFLECTIVITY", m_Cfg.rMask);
	mask->SetMaterialPropertiesTable(mpt);
	new G4LogicalBorderSurface("MaskSurface" + tag, padPV, maskPV, mask);

	const G4double area = sipmSV->GetCubicVolume() / m_ROThick;
	G4cout << "Read-out end " << tag << ": " << squares.size() << " SiPMs, sensitive area " << area / mm2
	       << " mm2 of " << 0.5 * m_Cfg.base * m_Cfg.height / mm2 << " mm2 (face)" << G4endl;
}

G4VPhysicalVolume* SiTHOptDetCon::Construct()
{
	const G4double b = m_Cfg.base, h = m_Cfg.height, L = m_Cfg.length;

	// World: air, photons reaching it are killed in the stepping action
	auto air = OpticalMaterial("OpticalAir", "G4_AIR", 1.0, -1.);
	auto worldSV = new G4Box("World", b / 2 + 2 * cm, h / 2 + 2 * cm, L / 2 + 2 * cm);
	auto worldLV = new G4LogicalVolume(worldSV, air, "World");
	m_WorldPV = new G4PVPlacement(nullptr, G4ThreeVector(), worldLV, "World", nullptr, false, 0, true);

	// Bar
	auto ej200 = OpticalMaterial("EJ200", "G4_PLASTIC_SC_VINYLTOLUENE", m_Cfg.nBar, m_Cfg.attBar);
	auto barSV = new G4ExtrudedSolid("Bar", m_Triangle, L / 2);
	auto barLV = new G4LogicalVolume(barSV, ej200, "Bar");
	auto barPV = new G4PVPlacement(nullptr, G4ThreeVector(), barLV, "Bar", worldLV, false, 0, true);

	// Teflon wrapping on every face not covered by a pad: polished bar surface
	// (specular reflection on micro-facets), air gap, Lambertian reflector
	auto wrap = new G4OpticalSurface("Teflon", unified, groundbackpainted, dielectric_dielectric, m_Cfg.sigmaAlpha);
	auto mpt = new G4MaterialPropertiesTable();
	AddFlat(mpt, "RINDEX", 1.0);
	AddFlat(mpt, "REFLECTIVITY", m_Cfg.rWrap);
	AddFlat(mpt, "SPECULARLOBECONSTANT", 1.0);
	AddFlat(mpt, "SPECULARSPIKECONSTANT", 0.0);
	AddFlat(mpt, "BACKSCATTERCONSTANT", 0.0);
	wrap->SetMaterialPropertiesTable(mpt);
	new G4LogicalBorderSurface("Teflon", barPV, m_WorldPV, wrap);

	// Read-out
	const G4double pitch = m_Cfg.sipm + m_Cfg.sipmGap;
	if ( m_Cfg.layout == "ONE" )
	{
		auto squares = AlongSide(-1, { -pitch / 2, pitch / 2 });
		auto right = AlongSide(+1, { -pitch / 2, pitch / 2 });
		squares.insert(squares.end(), right.begin(), right.end());
		PlaceReadout(-1, squares, worldLV);
	}
	else
	{
		auto squares = AlongSide(-1, { 0. });
		auto right = AlongSide(+1, { 0. });
		squares.insert(squares.end(), right.begin(), right.end());
		PlaceReadout(-1, squares, worldLV);
		PlaceReadout(+1, squares, worldLV);
	}
	return m_WorldPV;
}
