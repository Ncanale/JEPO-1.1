// File:	InteractionElasticF.hh
// Date:	13-Apr-2015 G.Macharashvili Dubna

#ifndef InteractionElasticF_hh
#define InteractionElasticF_hh 1

#include <fstream> 
#include <iomanip> 
using namespace std ;

#include <VInteractionGenerator.hh>

class InteractionElasticF : public VInteractionGenerator {

static	void	*ptr ;

public:

	InteractionElasticF(const char * = "InteractionElasticF") ;
virtual	~InteractionElasticF() ;

virtual	void	GenerateInteraction() ;
virtual	void	Init(G4ParticleDefinition *particleDefinition = 0,G4double kineticEnergy = 0.0,G4double t1 = 0.0,G4double t2 = 0.0 ) ;
static	void	*get() ;

protected:

	ifstream	in ;
	int		readEvents , acceptedEvents ;

	} ;

#endif

// e-o-f
