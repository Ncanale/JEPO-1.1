////////////////////////////////////////////////////////////////////////////////
//   JePoDetCon.cc for beamtest2017 of JEDI                                 //
//                                                                            //
//   Definitions of JePoDetCon class's member functions.                    //
// And it describes geometry of simulation.                                   //
//                                                                            //
//                    - 20. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) //
////////////////////////////////////////////////////////////////////////////////

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4Sphere.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4UIcommand.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Polycone.hh"
#include "G4ExtrudedSolid.hh"

#include "JePoDetCon.hh"


//////////////////////////////////////////////////
//   Constructor and destructor                 //
//////////////////////////////////////////////////
JePoDetCon::JePoDetCon(BT2017ConMan* CM)
	: m_CM(CM)
{
	ConstructMaterials();
	DefineDimensions();
}


JePoDetCon::~JePoDetCon()
{
	DestructMaterials();
}


//////////////////////////////////////////////////
//   Define dimensions                          //
//////////////////////////////////////////////////
void JePoDetCon::DefineDimensions()
{

	// World dimensions
	// // Laboratory size
	labX = m_CM -> GetWorldX(); // World x dimension
	labY = m_CM -> GetWorldY(); // World y dimension
	labZ = m_CM -> GetWorldZ(); // World z dimension

	// Target dimensions
	tarT = m_CM -> GetTarThi(); // Thickness along Y axis
	tarX = m_CM -> GetTarXle(); // length along X axis
	tarY = m_CM -> GetTarYle(); // length along Y axis
	tarxP= m_CM -> GetTarPoX(); // target X position
	taryP= m_CM -> GetTarPoY(); // target Y position

	// Crystral dimensions
	detE =  30.0 * mm; // Edge of square
	detL =  80.0 * mm; // Length of crystal
	detG =   0.3 * mm; // Gap between modules
	detX = m_CM -> GetDetPoX();
	detY = m_CM -> GetDetPoY();
	detZ = m_CM -> GetDetPoZ(); // Position
	detA = m_CM -> GetDetAng(); // Rotation
	detS = m_CM -> GetDetSet(); // Setup
	tefT =  50.0 * um; // Teflon thickness
	tedT =  50.0 * um; // Tedlar thickness
	kapT =  25.0 * um; // Kapton thickness

	// Plastic dimensions (It will measure dE)
// 	scXL = 130.0 * mm; // original size
// 	scYL = 100.0 * mm;
// 	scZL =  20.0 * mm;

	scXL = 150.0 * mm; // modified
	scYL =  90.0 * mm;
	scZL =  20.0 * mm;

	// Tracker dimensions (It will measure Particle Position)
	trS = m_CM -> GetTrSet(); // Setup
	trCon = m_CM -> GetTrackerConfig(); // Setup
 	trB = 60.0 * mm; // single prism dimensions
 	trH = 20.0 * mm;
  	//trL = 300.0 * mm;
 	trL = 7 * trB;
 	AlT =  0.01 * mm;
	//holX = 90.0 * mm;
	//holY = 90.0 * mm;
	holX = 0.01 * mm;
	holY = 0.01 * mm;
	layT = 2.0 * mm;

	// Configuration for B0 volume
	const	G4double	zl = 404.8/2 ;
	const	G4double	fl = 24.0 ; 
	const	G4double	RO = 254.0/2 ; 
	const	G4double	RT = 206.0/2 ;
	const	G4double	RI = RT-3.0 ;
	zz0 = new G4double [nz0] { -zl,  -(zl-fl), -(zl-fl), 0.0, +(zl-fl), +(zl-fl),   +zl } ;
	Rmin0 = new G4double [nz0] { RT,RI,RI,RI,RI,RI,RI } ;
	Rmax0 = new G4double [nz0] { RO,RO,RT,RT,RT,RO,RO } ;

	// Configuration small barrel (volume B1)
	const	G4double	vg = 469.9/2 ;
	const	G4double	vh = 401.5/2 ;
	zz1 = new G4double [nz1] {0.0,  24.0,  24.0, 122.0, 122.0, 150.0 } ;
	Rmin1 = new G4double [nz1] {200.0/2,  200.0/2,  200.0/2,  200.0/2,  200.0/2,  200.0/2 } ;
	Rmax1 = new G4double [nz1] {254.0/2,  254.0/2,  206.0/2,  206.0/2,  vg,  vg  } ;

	// Configuration large barrel (volume B2)
	zz2 = new G4double [nz2] { 0.0,  28.0,  28.0, 182.0, 182.0, 210.0 } ;
	Rmin2 = new G4double [nz2] {398.5/2,  398.5/2,  398.5/2,  398.5/2,  398.5/2,  398.5/2 } ;
	Rmax2 = new G4double [nz2] {     vg,       vg,       vh,       vh,       vg,       vg } ;

	// Configuration 88 mm diameter tube (volume B3)
	G4double zx3 = (88.9-67.0)*0.5/tan(16.82*deg/2) ;
	zz3 = new G4double [nz3] {  -8.0,   0.0,      zx3,    377.8,    377.8,  377.8+21.0 } ;
	Rmin3 = new G4double [nz3] { 67./2, 67./2, 88.9/2-2, 88.9/2-2, 88.9/2-2,  88.9/2-2 } ;
	Rmax3 = new G4double [nz3] { 71./2, 71./2,   88.9/2,   88.9/2,  203.2/2,   203.2/2 } ;

	// Configuration beam tube collar (volume Bv)
	const	G4double	vo = 71.0/2 ;
	zzv  = new G4double [nzv] {   -8.0,   -5.0,   -4.5,    -4.5,    -0.7,   -0.7,   -0.5,    0.0,    0.0,     0.8 } ;
	Rminv = new G4double [nzv] {     vo,     vo,     vo,  vo+1.8,  vo+1.8, vo+0.4, vo+0.4, vo+0.4, vo+0.4,  vo+0.4 } ;
	Rmaxv = new G4double [nzv] { vo+2.0, vo+3.0, vo+3.0,  vo+3.0,  vo+3.0, vo+3.0, vo+3.0, vo+5.0, vo+0.9,  vo+0.9 } ;

	// Configuration particle exit window (volume Bw)
	const	G4double windowThickness = 0.8 ;
	zzw  = new G4double [nzw] { -8.0,  0.0,     0.0,     windowThickness,  windowThickness,  20.0 } ;
	Rminw = new G4double [nzw] {   vh,   vh,  vo+0.9,  vo+0.9,   vh,  vh } ;
	Rmaxw = new G4double [nzw] {   vg,   vg,      vg,      vg,   vg,  vg } ;
}


//////////////////////////////////////////////////
//   Construct everything                       //
//////////////////////////////////////////////////
G4VPhysicalVolume* JePoDetCon::Construct()
{
	// World
	labSolid = new G4Box("labSolid", labX/2, labY/2, labZ/2);
	labLV = new G4LogicalVolume(labSolid, m_VacMat, "labLV");
	labPV = new G4PVPlacement(0, G4ThreeVector(), "labPV", labLV, 0, false, 0);

	// Perpendicular target
	target = new G4Box("target", tarX/2, tarY/2, tarT/2);
	tarLV = new G4LogicalVolume(target, m_TarMat, "tarLV");
	if ( m_CM -> GetUseTar() )
		new G4PVPlacement(0, G4ThreeVector(tarxP, taryP, 0), "tarPV", tarLV, labPV, false, 0);	

	// Material and VisAttribute initialization for symmetric volumes
	symVolMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_STAINLESS-STEEL") ;
	steelVis = new G4VisAttributes(G4Colour(0.75,0.75,0.75)) ;
	steelVis->SetVisibility(true) ;

	// construct symetric volumes
	ConstructSymmetricVolume("B0", 0);
	ConstructSymmetricVolume("B1", 1);
	ConstructSymmetricVolume("B2", 2);
	ConstructSymmetricVolume("B3", 3);
	ConstructSymmetricVolume("Bv", 4);
	ConstructSymmetricVolume("Bw", 5);

	// place them
	if(symVolConstructed[0] && m_CM->GetUseVB0())
		new G4PVPlacement(0, G4ThreeVector(0,0,symVolZpos[0]), symVolName[0], symVolLog[0], labPV, false, 0) ;
	if(symVolConstructed[1] && m_CM->GetUseVB1())
		new G4PVPlacement(0, G4ThreeVector(0,0,symVolZpos[1]), symVolName[1], symVolLog[1], labPV, false, 0) ;
	if(symVolConstructed[2] && m_CM->GetUseVB2())
		new G4PVPlacement(0, G4ThreeVector(0,0,symVolZpos[2]), symVolName[2], symVolLog[2], labPV, false, 0) ;
	if(symVolConstructed[3] && m_CM->GetUseVB3())
		new G4PVPlacement(0, G4ThreeVector(0,0,symVolZpos[3]), symVolName[3], symVolLog[3], labPV, false, 0) ;
	if(symVolConstructed[4] && m_CM->GetUseVBV())
		new G4PVPlacement(0, G4ThreeVector(0,0,symVolZpos[4]), symVolName[4], symVolLog[4], labPV, false, 0) ;
	if(symVolConstructed[5] && m_CM->GetUseVBW())
		new G4PVPlacement(0, G4ThreeVector(0,0,symVolZpos[5]), symVolName[5], symVolLog[5], labPV, false, 0) ;
	

	// Crystal detectors start here
	kaptonBase = new G4Box("kaptonBase", detE/2+tefT+tedT+kapT, detE/2+tefT+tedT+kapT, detL/2+tefT+tedT+kapT);
	tedlarBase = new G4Box("tedlarBase", detE/2+tefT+tedT,      detE/2+tefT+tedT,      detL/2+tefT+tedT);
	teflonBase = new G4Box("teflonBase", detE/2+tefT,           detE/2+tefT,           detL/2+tefT);
	crystal   = new G4Box("crystal", detE/2,              detE/2,                detL/2);
	teflon = new G4SubtractionSolid("teflon", teflonBase, crystal,    0, G4ThreeVector(0., 0., tefT));
	tedlar = new G4SubtractionSolid("tedlar", tedlarBase, teflonBase, 0, G4ThreeVector(0., 0., tedT));
	kapton = new G4SubtractionSolid("kapton", kaptonBase, tedlarBase, 0, G4ThreeVector(0., 0., kapT));
	cryRot = new G4RotationMatrix();
	cryRot -> rotateZ(-detA);


	// Left crystals
	for ( int i = 0; i < 3; i++ ) // L1 crystals
		                   ConstructCrystal("L", 69-10*i, 1, i+1, 3);
	for ( int i = 0; i < 5; i++ ) // L2 crystals
	{
		if      ( i < 4 )  ConstructCrystal("L", 78-10*i, 2, i+1, 5);
		else               ConstructCrystal("L",      35, 2, i+1, 5);
	}
	for ( int i = 0; i < 7; i++ ) // L3 crystals
	{
		if      ( i < 1 )  ConstructCrystal("L",      89, 3, i+1, 7);
		else if ( i < 5 )  ConstructCrystal("L", 87-10*i, 3, i+1, 7);
		else if ( i == 5 ) ConstructCrystal("L",      34, 3, i+1, 7);
		else               ConstructCrystal("L",      22, 3, i+1, 7);
	}
	for ( int i = 0; i < 7; i++ ) // L4 crystals
	{
		if      ( i < 1 )  ConstructCrystal("L",      88, 4, i+1, 7);
		else if ( i < 5 )  ConstructCrystal("L", 86-10*i, 4, i+1, 7);
		else if ( i == 5 ) ConstructCrystal("L",      33, 4, i+1, 7);
		else               ConstructCrystal("L",      21, 4, i+1, 7);
	}
	for ( int i = 0; i < 5; i++ ) // L5 crystals
	{
		if      ( i < 4 )  ConstructCrystal("L", 75-10*i, 5, i+1, 5);
		else               ConstructCrystal("L",      32, 5, i+1, 5);
	}
	
	// Right crystals
	for ( int i = 0; i < 3; i++ ) // R1 crystals
		                   ConstructCrystal("R", 50+10*i, 1, i+1, 3);
	for ( int i = 0; i < 5; i++ ) // R2 crystals
	{
		if      ( i < 4 )  ConstructCrystal("R", 41+10*i, 2, i+1, 5);
		else               ConstructCrystal("R",      84, 2, i+1, 5);
	}
	for ( int i = 0; i < 7; i++ ) // R3 crystals
	{
		if      ( i < 1 )  ConstructCrystal("R",      30, 3, i+1, 7);
		else if ( i < 5 )  ConstructCrystal("R", 32+10*i, 3, i+1, 7);
		else if ( i == 5 ) ConstructCrystal("R",      85, 3, i+1, 7);
		else               ConstructCrystal("R",      97, 3, i+1, 7);
	}
	for ( int i = 0; i < 7; i++ ) // R4 crystals
	{
		if      ( i < 1 )  ConstructCrystal("R",      31, 4, i+1, 7);
		else if ( i < 5 )  ConstructCrystal("R", 33+10*i, 4, i+1, 7);
		else if ( i == 5 ) ConstructCrystal("R",      86, 4, i+1, 7);
		else               ConstructCrystal("R",      98, 4, i+1, 7);
	}
	for ( int i = 0; i < 5; i++ ) // R5 crystals
	{
		if      ( i < 4 )  ConstructCrystal("R", 44+10*i, 5, i+1, 5);
		else               ConstructCrystal("R",      87, 5, i+1, 5);
	}
	
	// Up crystals
	for ( int i = 0; i < 3; i++ ) // U1 crystals
		                   ConstructCrystal("U",    37+i, 1, i+1, 3);
	for ( int i = 0; i < 5; i++ ) // U2 crystals
		                   ConstructCrystal("U",    24+i, 2, i+1, 5);
	for ( int i = 0; i < 7; i++ ) // U3 crystals
		                   ConstructCrystal("U",    13+i, 3, i+1, 7);
	for ( int i = 0; i < 7; i++ ) // U4 crystals
		                   ConstructCrystal("U",     5+i, 4, i+1, 7);
	for ( int i = 0; i < 5; i++ ) // U5 crystals
		                   ConstructCrystal("U",       i, 5, i+1, 5);
	// Down crystals
	for ( int i = 0; i < 3; i++ ) // D1 crystals
		                   ConstructCrystal("D",    82-i, 1, i+1, 3);
	for ( int i = 0; i < 5; i++ ) // D2 crystals
		                   ConstructCrystal("D",    95-i, 2, i+1, 5);
	for ( int i = 0; i < 7; i++ ) // D3 crystals
		                   ConstructCrystal("D",   106-i, 3, i+1, 7);
	for ( int i = 0; i < 7; i++ ) // D4 crystals
		                   ConstructCrystal("D",   114-i, 4, i+1, 7);
	for ( int i = 0; i < 5; i++ ) // D5 crystals
		                   ConstructCrystal("D",   119-i, 5, i+1, 5);
	// Cross crystals
	ConstructCrystal("C",  36, 1, 1, 4); // C1 crystals from here
	ConstructCrystal("C",  40, 1, 2, 4);
	ConstructCrystal("C",  83, 1, 3, 4);
	ConstructCrystal("C",  79, 1, 4, 4);
	ConstructCrystal("C",  23, 2, 1, 4); // C2 crystals from here
	ConstructCrystal("C",  29, 2, 2, 4);
	ConstructCrystal("C",  96, 2, 3, 4);
	ConstructCrystal("C",  90, 2, 4, 4);
	ConstructCrystal("C",  12, 3, 1, 4); // C3 crystals from here
	ConstructCrystal("C",  20, 3, 2, 4);
	ConstructCrystal("C", 107, 3, 3, 4);
	ConstructCrystal("C",  99, 3, 4, 4);

	// Place them
	for ( int i = 0; i < 120; i++ )
		if ( detS[i] && isConstructed[i] )
		{
			new G4PVPlacement(cryRot, cryPos[i], cryName[i]+"_cryPV", cryLV[i], labPV, false, 0);
			new G4PVPlacement(cryRot, tefPos[i], cryName[i]+"_tefPV", tefLV[i], labPV, false, 0);
			new G4PVPlacement(cryRot, tedPos[i], cryName[i]+"_tedPV", tedLV[i], labPV, false, 0);
			new G4PVPlacement(cryRot, kapPos[i], cryName[i]+"_kapPV", kapLV[i], labPV, false, 0);
		}

  
	// Plastic scintillator initialization
	scintillator = new G4Box("scintillator", scXL/2, scYL/2,  scZL/2);

  // construct scintillators
	ConstructScintillator("R", 0, m_CM -> GetExpand());
	ConstructScintillator("U", 1, m_CM -> GetExpand());
	ConstructScintillator("L", 2, m_CM -> GetExpand());
	ConstructScintillator("D", 3, m_CM -> GetExpand());

	// matrix to rotate scintillators
	for( int i = 0; i < 4; i++)
		pRot[i] = new G4RotationMatrix();

	// Place them
	for( int i = 0; i < 4; i++)
		if( sciIsConstructed[i] && m_CM -> GetUseSci()){
			pRot[i]->rotateZ(i * 90 * deg);
			new G4PVPlacement(pRot[i], vector[i],  "LV_" + sciName[i], sciLV[i], labPV, false, 0);
		}
  

  // construct trackers
  ConstructTracker();
	for( int i = 0; i < 28; i++) PlanTracker(i, m_CM -> GetTranslate());

	// Place them
	for( int i = 0; i < 28; i++) if( trS[i] && trIsConstructed[i]) PlaceTracker(i,m_CM->GetTranslate());
	G4bool AtrS = 0;
	for( int i = 0; i < 28; i++) AtrS = AtrS || trS[i];
	if(AtrS)
	{
		new G4PVPlacement(0, pLayPosF, "playF", pLayLV, labPV, false, 0);
		new G4PVPlacement(0, pLayPosB, "playB", pLayLV, labPV, false, 0);
	}
	
	return labPV;
}

void JePoDetCon::ConstructMaterials()
{
	const G4double labTemp = 300.0 * kelvin;
	// Elements to be used to construct materials
	m_ElH  = new G4Element( "Hydrogen",  "H",  1,   1.00794*g/mole);
	m_ElC  = new G4Element(   "Carbon",  "C",  6,  12.011  *g/mole);
	m_ElN  = new G4Element( "Nitrogen",  "N",  7,  14.00674*g/mole);
	m_ElO  = new G4Element(   "Oxygen",  "O",  8,  15.9994 *g/mole);
	m_ElF  = new G4Element( "Fluorine",  "F",  9,  18.9984 *g/mole);
	m_ElMg = new G4Element("Magnesium", "Mg", 12,  24.3050 *g/mole);
	m_ElAl = new G4Element( "Aluminum", "Al", 13,  26.98154*g/mole);
	m_ElSi = new G4Element(  "Silicon", "Si", 14,  28.0855 *g/mole);
	m_ElAr = new G4Element(    "Argon", "Ar", 18,  39.948  *g/mole);
	m_ElCr = new G4Element("Chromuium", "Cr", 24,  51.9961 *g/mole);
	m_ElFe = new G4Element(     "Iron", "Fe", 26,  55.845  *g/mole);
	m_ElNi = new G4Element(   "Nickel", "Ni", 28,  58.9634 *g/mole);
	m_ElCu = new G4Element(   "Copper", "Cu", 29,  63.546  *g/mole);
	m_ElZn = new G4Element(     "Zinc", "Zn", 30,  65.38   *g/mole);
	m_ElY  = new G4Element(  "Yttrium",  "Y", 39,  88.90585*g/mole);
	m_ElSn = new G4Element(      "Tin", "Sn", 50, 118.71   *g/mole);
	m_ElCe = new G4Element(   "Cerium", "Ce", 58, 140.116  *g/mole);
	m_ElLu = new G4Element( "Lutetium", "Lu", 71, 176.944  *g/mole);

	// Air material
	m_AirMat = new G4Material("Air", 1.1839e-3*g/cm3, 3, kStateGas, labTemp); // Density at 1 atm
	m_AirMat -> AddElement(m_ElN, 75.47/99.95);
	m_AirMat -> AddElement(m_ElO, 23.20/99.95);
	m_AirMat -> AddElement(m_ElAr, 1.28/99.95);

	// Target material
	G4String targetMaterial = m_CM -> GetTarMat();
	if ( targetMaterial == "C" )
	{
		m_TarMat = new G4Material("Graphite", 1.7*g/cm3, 3, kStateSolid, labTemp);
		m_TarMat -> AddElement(m_ElC, 99.0/100.0);
		m_TarMat -> AddElement(m_ElN,  0.7/100.0);
		m_TarMat -> AddElement(m_ElO,  0.3/100.0);
	}
	else if ( targetMaterial == "Mg" )
	{
		m_TarMat = new G4Material("Magnesium", 1.738*g/cm3, 1, kStateSolid, labTemp);
		m_TarMat -> AddElement(m_ElMg, 1.0);
	}
	else if ( targetMaterial == "Al" )
	{
		m_TarMat = new G4Material("Aluminum", 2.70*g/cm3, 1, kStateSolid, labTemp);
		m_TarMat -> AddElement(m_ElAl, 1.0);
	}
	else if ( targetMaterial == "Sn" )
	{
		m_TarMat = new G4Material("Tin", 5.769*g/cm3, 1, kStateSolid, labTemp); // Gray tin?
		m_TarMat -> AddElement(m_ElSn, 1.0);
	}
	else if ( targetMaterial == "Si" )
	{
		m_TarMat = new G4Material("Silicon", 2.329*g/cm3, 1, kStateSolid, labTemp);
		m_TarMat -> AddElement(m_ElSi, 1.0);
	}
	else if ( targetMaterial == "Ni" )
	{
		m_TarMat = new G4Material("Nickel", 8.908*g/cm3, 1, kStateSolid, labTemp);
		m_TarMat -> AddElement(m_ElNi, 1.0);
	}
	else if ( targetMaterial == "Zn" )
	{
		m_TarMat = new G4Material("Zinc", 7.14*g/cm3, 1, kStateSolid, labTemp);
		m_TarMat -> AddElement(m_ElZn, 1.0);
	}
	else if ( targetMaterial == "CH2" )
	{
		m_TarMat = new G4Material("Polyethelene", 0.92*g/cm3, 2, kStateSolid, labTemp);
		m_TarMat -> AddElement(m_ElC, 1);
		m_TarMat -> AddElement(m_ElH, 2);
	}
	else
	{
		std::stringstream message;
		message << "Could not find target material with name " << targetMaterial;
		G4Exception("JePoDetCon::ConstructMaterials()", "", FatalException, message.str().c_str());
	}

	// Crystal scintillator material
	m_DetMat = new G4Material("LYSO", 7.1*g/cm3, 5, kStateSolid, labTemp);
	m_DetMat -> AddElement(m_ElLu, 71.43/100.0);
	m_DetMat -> AddElement(m_ElY,   4.03/100.0);
	m_DetMat -> AddElement(m_ElSi,  6.37/100.0);
	m_DetMat -> AddElement(m_ElO,  18.14/100.0);
	m_DetMat -> AddElement(m_ElCe,  0.02/100.0);

	// Plastic scintillator material
	m_CouMat = new G4Material("PlasticScint", 1.023*g/cm3, 2, kStateSolid, labTemp);
	m_CouMat -> AddElement(m_ElC, 10);
	m_CouMat -> AddElement(m_ElH, 11);

	// Pipe material
	m_PipMat = new G4Material("StainlessSteel", 8.03*g/cm3, 3, kStateSolid, labTemp);
	m_PipMat -> AddElement(m_ElFe, 70.0/100.0);
	m_PipMat -> AddElement(m_ElCr, 20.0/100.0);
	m_PipMat -> AddElement(m_ElNi, 10.0/100.0);

	// Vacuum material
	m_VacMat = new G4Material("Vacumm", 1.1839e-10*g/cm3, 3, kStateGas, labTemp);
	m_VacMat -> AddElement(m_ElN, 75.47/99.95);
	m_VacMat -> AddElement(m_ElO, 23.20/99.95);
	m_VacMat -> AddElement(m_ElAr, 1.28/99.95);

	// Teflon material
	m_TefMat = new G4Material("Teflon", 2.2*g/cm3, 2, kStateSolid, labTemp);
	m_TefMat -> AddElement(m_ElC, 1);
	m_TefMat -> AddElement(m_ElF, 2);

	// Tedlar material
	m_TedMat = new G4Material("Tedlar", 1.545*g/cm3, 3, kStateSolid, labTemp);
	m_TedMat -> AddElement(m_ElC, 2);
	m_TedMat -> AddElement(m_ElH, 3);
	m_TedMat -> AddElement(m_ElF, 1);

	// Kapton mateiral
	m_KapMat = new G4Material("Kapton", 1.39*g/cm3, 3, kStateSolid, labTemp);
	m_KapMat -> AddElement(m_ElC, 5);
	m_KapMat -> AddElement(m_ElO, 2);
	m_KapMat -> AddElement(m_ElH, 4);

	// Ar+CO2 gas material
	m_GasMat = new G4Material("Gas", 1.184*kg/m3, 3, kStateGas, labTemp);
	m_GasMat -> AddElement(m_ElAr, 12);
	m_GasMat -> AddElement(m_ElC,   1);
	m_GasMat -> AddElement(m_ElO,   2);

	// PCB material
	m_PCBMat = new G4Material("PCB", 2.0*g/cm3, 2, kStateSolid, labTemp);
	m_PCBMat -> AddElement(m_ElSi, 1);
	m_PCBMat -> AddElement(m_ElO,  2);

	// Copper coating material
	m_CoaMat = new G4Material("Coating", 8.96*g/cm3, 1, kStateSolid, labTemp);
	m_CoaMat -> AddElement(m_ElCu, 1);

	// Aluminium coating material
	m_AlCoaMat = new G4Material("Aluminium Coating", 2.70*g/cm3, 1, kStateSolid, labTemp);
	m_AlCoaMat -> AddElement(m_ElAl, 1);

	// Material of Plastic Layer around Trackers
	m_LayMat = new G4Material("Polyethelene", 0.92*g/cm3, 2, kStateSolid, labTemp);
	m_LayMat -> AddElement(m_ElC, 1);
	m_LayMat -> AddElement(m_ElH, 2);
}

void JePoDetCon::DestructMaterials()
{
	delete m_CoaMat;
	delete m_PCBMat;
	delete m_GasMat;
	delete m_KapMat;
	delete m_TedMat;
	delete m_TefMat;
	delete m_VacMat;
	delete m_PipMat;
	delete m_CouMat;
	delete m_TarMat;
	delete m_AirMat;

	delete m_ElLu;
	delete m_ElCe;
	delete m_ElSn;
	delete m_ElY;
	delete m_ElZn;
	delete m_ElCu;
	delete m_ElNi;
	delete m_ElFe;
	delete m_ElCr;
	delete m_ElAr;
	delete m_ElSi;
	delete m_ElAl;
	delete m_ElMg;
	delete m_ElF;
	delete m_ElO;
	delete m_ElN;
	delete m_ElC;
	delete m_ElH;
}

//////////////////////////////////////////////////
//   Construct a crystal                        //
//   This function is used to repeat            //
// construction of crystals in                  //
// JePoDetCon::Contruct function.             //
//////////////////////////////////////////////////
void JePoDetCon::ConstructCrystal(G4String direction,
	G4int detID, G4int thetaID, G4int phiID, G4int maxPhi)
{
	// Construct name of this module
	cryName[detID]  = direction + G4UIcommand::ConvertToString(thetaID);
	cryName[detID] += "_" + G4UIcommand::ConvertToString(phiID);
	cryName[detID] += "_" + G4UIcommand::ConvertToString(maxPhi);

	// Define logical volume
	kapLV[detID] = new G4LogicalVolume(kapton,  m_KapMat, "kapLV");
	tedLV[detID] = new G4LogicalVolume(tedlar,  m_TedMat, "tedLV");
	tefLV[detID] = new G4LogicalVolume(teflon,  m_TefMat, "tefLV");
	cryLV[detID] = new G4LogicalVolume(crystal, m_DetMat, cryName[detID]);

	// Where is it?
	if      ( direction == "L" )
		cryPos[detID] = G4ThreeVector(30.3*(thetaID+1)*mm, 30.3*(phiID-maxPhi/2-1)*mm, detZ + detL/2);
	else if ( direction == "R" )
		cryPos[detID] = G4ThreeVector(- 30.3*(thetaID+1)*mm, 30.3*(phiID-maxPhi/2-1)*mm, detZ + detL/2);
	else if ( direction == "U" )
		cryPos[detID] = G4ThreeVector(30.3*(phiID-maxPhi/2-1)*mm, 30.3*(thetaID+1)*mm, detZ + detL/2);
	else if ( direction == "D" )
		cryPos[detID] = G4ThreeVector(30.3*(phiID-maxPhi/2-1)*mm, - 30.3*(thetaID+1)*mm, detZ + detL/2);
	else if ( direction == "C" )
		cryPos[detID] = G4ThreeVector(std::sqrt(2.0)*std::cos((90.0*phiID-45.0)*deg)*30.3*(thetaID+1)*mm,
		                              std::sqrt(2.0)*std::sin((90.0*phiID-45.0)*deg)*30.3*(thetaID+1)*mm,
		                              detZ + detL/2);
	// Apply translation
	cryPos[detID] = cryPos[detID] + G4ThreeVector(m_CM -> GetDetPoX(), m_CM -> GetDetPoY(), 0.);

	// Rotate it
	cryPos[detID].rotateZ(detA);

	// Wrap this crystal
	tefPos[detID] = cryPos[detID] - G4ThreeVector(0., 0., tefT);
	tedPos[detID] = cryPos[detID] - G4ThreeVector(0., 0., tefT+tedT);
	kapPos[detID] = cryPos[detID] - G4ThreeVector(0., 0., tefT+tedT+kapT);

	// Beautiful colours
	if ( direction == "L" )
		cryLV[detID] -> SetVisAttributes(new G4VisAttributes(G4Colour::Cyan()));
	if ( direction == "R" )
		cryLV[detID] -> SetVisAttributes(new G4VisAttributes(G4Colour::Cyan()));
	if ( direction == "U" )
		cryLV[detID] -> SetVisAttributes(new G4VisAttributes(G4Colour::Magenta()));
	if ( direction == "D" )
		cryLV[detID] -> SetVisAttributes(new G4VisAttributes(G4Colour::Magenta()));
	if ( direction == "C" )
		cryLV[detID] -> SetVisAttributes(new G4VisAttributes(G4Colour::Green()));

	// wrapping 	
	tefLV[detID] -> SetVisAttributes(new G4VisAttributes(G4Colour::White()));
	tedLV[detID] -> SetVisAttributes(new G4VisAttributes(G4Colour::Gray()));
	kapLV[detID] -> SetVisAttributes(new G4VisAttributes(G4Colour::Yellow()));

	// Done!
	isConstructed[detID] = true;
}

void JePoDetCon::ConstructScintillator(G4String direction, G4int sciId, G4double expand)
{	
	// construct name of scintillator
	sciName[sciId] = "dE_" + direction;

	// initialize logical volume
	sciLV[sciId] = new G4LogicalVolume(scintillator, m_CouMat, sciName[sciId]);
	sciLV[sciId]-> SetVisAttributes(new G4VisAttributes(G4Color::Brown()));

	// Define position of scintillator
	if(direction == "R")
		vector[sciId] = G4ThreeVector(1.5*detE + detG + scXL/2 + expand, - (scYL/2 - 1.5*detE - detG), detZ - scZL/2 - 1.*mm);
	else if(direction == "L")
		vector[sciId] = G4ThreeVector(- (1.5*detE + detG + scXL/2 + expand), scYL/2 - 1.5*detE - detG, detZ - scZL/2 - 1.*mm);
	else if(direction == "U")
		vector[sciId] = G4ThreeVector((scYL/2 - 1.5*detE - detG), 1.5*detE + detG + scXL/2 + expand, detZ - scZL/2 - 1.*mm);
	else if(direction == "D")
		vector[sciId] = G4ThreeVector(-(scYL/2 - 1.5*detE - detG), -(1.5*detE + detG + scXL/2 + expand), detZ - scZL/2 - 1.*mm);
	

	// Done!
	sciIsConstructed[sciId] = true;
}

void JePoDetCon::ConstructTracker()
{
	// Define configuration for a structure
  G4double tanth = trH / (trB/2);
  G4double costh = 1 / sqrt(1 + tanth*tanth);
  G4double sinth = sqrt(1 - costh*costh);
  G4double iH = AlT / costh;
  G4double iB = AlT/tanth + AlT/sinth;
  G4double iO = (iH - AlT) / 2;

	// Construct shape of detector and cover
  std::vector<G4TwoVector> tBar,tCover;
  tBar.push_back(G4TwoVector(0 , trH/2-iO));
  tBar.push_back(G4TwoVector(-trB/2 , iO-trH/2));
  tBar.push_back(G4TwoVector(trB/2 , iO-trH/2));
  tCover.push_back(G4TwoVector(0 , iH+trH/2-iO));
  tCover.push_back(G4TwoVector(-iB-trB/2 , -iO-AlT-trH/2));
  tCover.push_back(G4TwoVector(iB+trB/2 , -iO-AlT-trH/2));
  G4TwoVector off1(0,0), off2(0,0);
  tracker_bar = new G4ExtrudedSolid("tracker_bar",tBar,trL/2,off1,1.,off2,1.);
  G4ExtrudedSolid* tracker_cover_solid = new G4ExtrudedSolid("tracker_cover_solid",tCover,trL/2,off1,1.,off2,1.);
	//tracker_bar = new G4SubtractionSolid("tracker_bar",tracker_bar_solid,hole);
	//G4SubtractionSolid* tracker_cover_full = new G4SubtractionSolid("tracker_cover_full",tracker_cover_solid,tracker_bar);
	tracker_cover = new G4SubtractionSolid("tracker_cover",tracker_cover_solid,tracker_bar);

	// initialize logical volume
	tcovLV = new G4LogicalVolume(tracker_cover, m_AlCoaMat, "tracker_cover");
  tbarLV = new G4LogicalVolume(tracker_bar, m_CouMat, "tracker_bar");
	tcovLV-> SetVisAttributes(new G4VisAttributes(G4Color(1.0, 1.0, 1.0, 0.2)));
	tbarLV-> SetVisAttributes(new G4VisAttributes(G4Color(0.0, 1.0, 0.0, 0.05)));

	// initialize the hole and the plastic layers
	holZ = 2*(iH + trH + AlT) + 3.0;
	hole = new G4Box("hole", holX/2, holY/2, holZ/2);
	G4Box* pLay_full = new G4Box("play_full", trL/2, trL/2, layT/2);
	pLay = new G4SubtractionSolid("play",pLay_full,hole);
	pLayLV = new G4LogicalVolume((trCon ? pLay : pLay_full), m_LayMat, "playLV");
	pLayLV-> SetVisAttributes(new G4VisAttributes(G4Color(1.0, 1.0, 0.0, 0.1)));
	pLayPosF = G4ThreeVector((trCon ? m_CM->GetTranslate() : 0),0,detZ-layT/2-2*(iH+trH+AlT+0.01)-30.0);
	pLayPosB = G4ThreeVector((trCon ? -m_CM->GetTranslate() : 0),0,detZ+layT/2-30.0+0.01);
}

void JePoDetCon::PlanTracker(G4int trID, G4double translate)
{	
	// construct name of scintillator
  nTr = (trID % 14 ? nTr : 0);
  //nTr = nTr + trS[trID];
  nTr = nTr + 1;
	trName[trID] = (trID / 14 ? "B_" : "F_") + ((nTr>9 ? "" : "0") + G4UIcommand::ConvertToString(nTr));
  
	// Define configuration for a structure
  G4double tanth = trH / (trB/2);
  G4double costh = 1 / sqrt(1 + tanth*tanth);
  G4double sinth = sqrt(1 - costh*costh);
  G4double iH = AlT / costh;
  G4double iB = AlT/tanth + AlT/sinth;
  //G4double iO = (iH - AlT) / 2;

  int ff, fb, bf, bb;
  ff = (1 - ((trID / 7) % 2)) * (1 - (trID / 14));
  fb = ((trID / 7) % 2) * (1 - (trID / 14));
  bf = (1 - ((trID / 7) % 2)) * (trID / 14);
  bb = ((trID / 7) % 2) * (trID / 14);

  // Define position of tracker
  G4double posX, posY, posZ, f_off;
  f_off = 20 + 1.336 -0.038 -0.122 ; //after some iteration of diff_peak.py
  if(trCon)     //  For perpendicular configuration
  {
    posX = translate * (ff + fb - bf - bb) + (-15 + (3.5 - (trID % 7) - bf*0.5) * (2*iB + trB)) * (bf + bb);
    posY = (-15 + (3.5 - (trID % 7) - ff*0.5) * (2*iB + trB)) * (ff + fb);
  }
  else          //  For parallel configuration
  {
    posX = 0;
    posY = (3.5 - (trID % 7) - (ff + bf)*0.5) * (2*iB + trB) + (bf + bb)*translate + (ff + fb)*f_off;
  }
  posZ = detZ - (0.5 + ff + fb)*(iH + trH + AlT + 0.01) - 30.0;
  posH = G4ThreeVector(0, 0, detZ - (iH + trH + AlT + 0.01) - 30.0);
  trPos[trID] = G4ThreeVector(posX,posY,posZ);
  
	// Define rotation of tracker
  tRot[trID] = new G4RotationMatrix;
	RotH = new G4RotationMatrix;
  tRot[trID] -> rotateX(-M_PI/2 * (ff - fb + bf - bb));
  tRot[trID] -> rotateY(-M_PI/2 * (trCon ? (ff + fb) : 1));    //  Y-rotation depends on configuration

  // Done!
	trIsConstructed[trID] = true;
}

void JePoDetCon::PlaceTracker(G4int trID, G4double translate)
{
	G4bool cutH = (trID%14 == 2) || (trID%14 == 3) || (trID%14 == 10) || (trID%14 == 11);
	//G4bool cutE = (trID%14 == 0) || (trID%14 == 6) || (trID%14 == 7) || (trID%14 == 13);
	//G4bool split = (trID%14 == 3);
	if(trCon && cutH)// || cutE)
	{
		G4ThreeVector trns((1 - 2*(trID / 14)) * translate, 0, 0);
		G4RotationMatrix i_rot = *tRot[trID];
		trns = posH+trns-trPos[trID];
		trns.transform(i_rot);
		i_rot.invert();
		G4SubtractionSolid* tracker_bar_cut = new G4SubtractionSolid("tracker_bar_cut",tracker_bar,hole,&i_rot,trns);
		G4SubtractionSolid* tracker_cover_cut = new G4SubtractionSolid("tracker_cover_cut",tracker_cover,hole,&i_rot,trns);
		G4LogicalVolume* barLV = new G4LogicalVolume(tracker_bar_cut, m_CouMat, "tracker_bar_cut" + G4UIcommand::ConvertToString(trID));
		G4LogicalVolume* covLV = new G4LogicalVolume(tracker_cover_cut, m_AlCoaMat, "tracker_cover_cut" + G4UIcommand::ConvertToString(trID));
		covLV-> SetVisAttributes(tcovLV->GetVisAttributes());
		barLV-> SetVisAttributes(tbarLV->GetVisAttributes());
		new G4PVPlacement(tRot[trID], trPos[trID], trName[trID], barLV, labPV, false, 0);
		new G4PVPlacement(tRot[trID], trPos[trID], trName[trID] + "_cover", covLV, labPV, false, 0);
	}
	else
	{
		new G4PVPlacement(tRot[trID], trPos[trID], trName[trID], tbarLV, labPV, false, 0);
		new G4PVPlacement(tRot[trID], trPos[trID], trName[trID] + "_cover", tcovLV, labPV, false, 0);
	}
}

void JePoDetCon::ConstructSymmetricVolume(G4String name, G4int symVolID, G4double f1, G4double f2){
	// define name of symmetric volume
	symVolName[symVolID] = name + "_steel";											

	// Define configuration for a structure
	G4int num = 0;
	G4double	*zz = new G4double[1] {0};
	G4double	*Rmin = new G4double[1] {0}; 
	G4double	*Rmax = new G4double[1] {0};
	if(name == "B0"){
		num = nz0; zz = zz0; Rmin = Rmin0; Rmax = Rmax0;
	}else if(name == "B1"){
		num = nz1; zz = zz1; Rmin = Rmin1; Rmax = Rmax1;
	}else if(name == "B2"){
		num = nz2; zz = zz2; Rmin = Rmin2; Rmax = Rmax2;
	}else if(name == "B3"){
		num = nz3; zz = zz3; Rmin = Rmin3; Rmax = Rmax3;
	}else if(name == "Bv"){
		num = nzv; zz = zzv; Rmin = Rminv; Rmax = Rmaxv;
	}else if(name == "Bw"){
		num = nzw; zz = zzw; Rmin = Rminw; Rmax = Rmaxw;
	}

	// construcs symmetric volume
	symVol[symVolID] = new G4Polycone(name, f1, f2-f1, num, zz, Rmin, Rmax) ;
 	symVolLog[symVolID] = new G4LogicalVolume(symVol[symVolID], symVolMaterial, symVolName[symVolID]) ;
	symVolLog[symVolID]->SetVisAttributes(steelVis) ;

	// define the position of next simetric cristal
	if(name == "B0")
		symVolZpos[symVolID + 1] = symVolZpos[symVolID] + (zz[num - 1] - zz[0])/2;
	else if(name == "B1")
		symVolZpos[symVolID + 1] = symVolZpos[symVolID] + (zz[num - 1] - zz[0]);
	else if(name == "B2")
		symVolZpos[symVolID + 1] = symVolZpos[symVolID] + (zz[num - 1] - zz[0]);
	else if(symVolID != 5)
		symVolZpos[symVolID + 1] = symVolZpos[symVolID];
	

	// is constructed
	symVolConstructed[symVolID] = true;
}

