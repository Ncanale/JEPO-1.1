////////////////////////////////////////////////////////////////////////////////
//   SiTHOptActIni.hh for the SiTH optical light-collection study             //
////////////////////////////////////////////////////////////////////////////////

#ifndef SITHOPTACTINI_H
#define SITHOPTACTINI_H

#include "G4VUserActionInitialization.hh"

#include "SiTHOptConfig.hh"

class SiTHOptActIni : public G4VUserActionInitialization
{
  public:
	SiTHOptActIni(const SiTHOptConfig& cfg) : m_Cfg(cfg) {}
	virtual ~SiTHOptActIni() {}

	virtual void BuildForMaster() const;
	virtual void Build() const;

  private:
	const SiTHOptConfig& m_Cfg;
};

#endif
