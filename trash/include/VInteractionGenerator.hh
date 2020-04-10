// File:	VInteractionGenerator.hh
// Date:	14-Apr-2015 G.Macharashvili Dubna

#ifndef VInteractionGenerator_hh
#define VInteractionGenerator_hh 1

#include <G4ThreeVector.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>

class VInteractionGenerator {

static	const	G4int		maximumTracks = 12 ;

private:

public:

	VInteractionGenerator(const char * name = "undef.generator") ;
virtual	~VInteractionGenerator() ;

const	G4String	&GetName() const { return name ; }

virtual	void		GenerateInteraction() = 0 ;	// interaction generator ...
//--virtual	void		*get() = 0 ;			// to create object ...

	void		SetThetaLimits(G4double t1 = 0,G4double t2 = 25.0 * CLHEP::deg) { minTheta = t1 ; maxTheta = t2 ; }
	void		SetPhi(G4double Xphi,G4int trk = 0) { phi[trk] = Xphi ; }

	G4int		GetNumberOfTracks() const { return num_of_tracks ; }
G4ParticleDefinition	*GetParticleDefinition(G4int trk) const { return def[trk] ; }
	G4double	GetTheta(G4int trk) const { return theta[trk] ; }
	G4double	GetPhi(G4int trk) const { return phi[trk] ; }
	G4double	GetKineticEnergy(G4int trk) const { return ek[trk] ; }

// before generation:
virtual	void		Init(G4ParticleDefinition *_particleDefinition = 0,G4double _kineticEnergy = 0,G4double _theta1 = 0,G4double _theta2 = 0) { ; }
//                                   ^  for single particle ^            ^  for fixed energy [MeV] ^     ^ for anglular interval [t*deg] ^
	void		SetBeam(G4ParticleDefinition *_particleDefinition,G4ThreeVector &_momentum) ;
	void		SetTarget(G4ParticleDefinition *_particleDefinition) ;

//	G4double	GetBeamEnergy() const { return beamEnergy ; } // do we need these ????
//	G4double	GetBeamAngle() const { return beamAngle ; }

protected:

	G4String		name ;			// the derived generator class name ...
	G4ParticleDefinition	*particle ;		// default particle definition ...
	G4double		kineticEnergy ;		// energy lower limit [MeV] ...
	G4double		minTheta , maxTheta ;	// theta limits [rad] ...

	G4ParticleDefinition	*beam , *target ;
	G4ThreeVector		beamMomentum ;
	
	G4int			num_of_tracks ;		// the data have to be filled in by derived class ...
	G4ParticleDefinition*	def[maximumTracks] ;	// ...
	G4double		theta[maximumTracks] ;	// lab. theta [rad]
	G4double		phi[maximumTracks] ;	// [rad]. default == 1000.
	G4double		ek[maximumTracks] ;	// kinetic energy [MeV]
	
private:
  
	} ;

#endif

// e-o-f