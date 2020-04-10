// File:	InteractionInelasticF.hh
// Date:	13-Apr-2015 G.Macharashvili Dubna

#ifndef InteractionInelasticF_hh
#define InteractionInelasticF_hh 1

#include <fstream> 
#include <iomanip> 
using namespace std ;

#include <VInteractionGenerator.hh>

class InteractionInelasticF : public VInteractionGenerator {

static	void	*ptr ;

private:

	G4double		kineticEnergy ;		// external. initialized with Init() ...
	G4ParticleDefinition	*particle ;		// external. initialized with Init() ...
	G4double		theta1 , theta2 ;	// external. initialized with Init() ...

private:

public:

	InteractionInelasticF(const char * = "InteractionInelasticF") ;
virtual	~InteractionInelasticF() ;

virtual	void	GenerateInteraction() ;
virtual	void	Init(G4ParticleDefinition *particleDefinition = 0,G4double kineticEnergy = 0,G4double t1 = -0.001,G4double t2 = -0.001 ) ;
static	void	*get() ;

protected:

	ifstream	in ;
	G4ParticleDefinition	*p , *d , *n , *a ;

	} ;

#endif

// e-o-f
