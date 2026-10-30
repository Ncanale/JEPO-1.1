////////////////////////////////////////////////////////////////////////////////
//   SiTHOptActIni.cc for the SiTH optical light-collection study             //
////////////////////////////////////////////////////////////////////////////////

#include "SiTHOptActIni.hh"

#include "SiTHOptPriGen.hh"
#include "SiTHOptRunAct.hh"
#include "SiTHOptSteAct.hh"

void SiTHOptActIni::BuildForMaster() const
{
	SetUserAction(new SiTHOptRunAct(m_Cfg));
}

void SiTHOptActIni::Build() const
{
	auto runAct = new SiTHOptRunAct(m_Cfg);
	SetUserAction(runAct);
	SetUserAction(new SiTHOptPriGen(m_Cfg, runAct));
	SetUserAction(new SiTHOptSteAct(runAct));
}
