#ifndef BT2017ANA_h
#define BT2017ANA_h 1

////////////////////////////////////////////////////////////////////////////////
//   BT2017Ana.hh for beamtest2017 of JEDI                                    //
//                                                                            //
//   Really simple, isn't it? It just set analysis tool. We will use ROOT.    //
// We can use ROOT inside GEANT using g4root which is a kind of embeded ROOT  //
// in GEANT. Of course, we can include ROOT in our system, but we have to fix //
// CMakeList.txt to do this.                                                  //
//                                                                            //
//                    - 21. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) //
////////////////////////////////////////////////////////////////////////////////

#include "G4Version.hh"
// g4root.hh was removed in Geant4 11: use the ROOT analysis manager directly
#if G4VERSION_NUMBER >= 1100
#include "G4RootAnalysisManager.hh"
using G4AnalysisManager = G4RootAnalysisManager;
#else
#include "g4root.hh"
#endif
//#include "g4xml.hh"

#endif
