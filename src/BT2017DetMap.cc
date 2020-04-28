////////////////////////////////////////////////////////////////////////////////
//   BT2017DetMap.cc for beamtest2017 of JEDI                                 //
//                                                                            //
//   Definitions of BT2017DetMap class's member functions                     //
//                                                                            //
//                    - 22. Nov. 2017. Hoyong Jeong (hyjeong@hep.korea.ac.kr) //
////////////////////////////////////////////////////////////////////////////////

#include "BT2017DetMap.hh"

//////////////////////////////////////////////////
//   Constructors and destructor                //
//////////////////////////////////////////////////
BT2017DetMap::BT2017DetMap()
{
  Init();
}

BT2017DetMap::~BT2017DetMap()
{
}


//////////////////////////////////////////////////
//   Initialization                             //
//////////////////////////////////////////////////
void BT2017DetMap::Init()
{
  m_DetNameFromDetID.clear();
  m_DetIDFromDetName.clear();
  m_dENameFromdEID.clear();
  m_dEIDFromdEName.clear();

  // Mapping from ID to name
  m_DetNameFromDetID[0]   = "U5_1_5"; // top row 6
  m_DetNameFromDetID[1]   = "U5_2_5";
  m_DetNameFromDetID[2]   = "U5_3_5"; 
  m_DetNameFromDetID[3]   = "U5_4_5";
  m_DetNameFromDetID[4]   = "U5_5_5";
    
  m_DetNameFromDetID[5]   = "U4_1_7"; // top row 6
  m_DetNameFromDetID[6]   = "U4_2_7"; 
  m_DetNameFromDetID[7]   = "U4_3_7";
  m_DetNameFromDetID[8]   = "U4_4_7"; 
  m_DetNameFromDetID[9]   = "U4_5_7";    
  m_DetNameFromDetID[10]  = "U4_6_7"; 
  m_DetNameFromDetID[11]  = "U4_7_7";
    
  m_DetNameFromDetID[12]  = "C3_1_4"; // top row 5
  m_DetNameFromDetID[13]  = "U3_1_7";
  m_DetNameFromDetID[14]  = "U3_2_7"; 
  m_DetNameFromDetID[15]  = "U3_3_7";
  m_DetNameFromDetID[16]  = "U3_4_7"; 
  m_DetNameFromDetID[17]  = "U3_5_7";
  m_DetNameFromDetID[18]  = "U3_6_7"; 
  m_DetNameFromDetID[19]  = "U3_7_7";    
  m_DetNameFromDetID[20]  = "C3_2_4";
    
  m_DetNameFromDetID[21]  = "L4_7_7"; // top row 4
  m_DetNameFromDetID[22]  = "L3_7_7";     
  m_DetNameFromDetID[23]  = "C2_1_4";	
  m_DetNameFromDetID[24]  = "U2_1_5"; 
  m_DetNameFromDetID[25]  = "U2_2_5";
  m_DetNameFromDetID[26]  = "U2_3_5"; 
  m_DetNameFromDetID[27]  = "U2_4_5";
  m_DetNameFromDetID[28]  = "U2_5_5";     
  m_DetNameFromDetID[29]  = "C2_2_4";
	
  m_DetNameFromDetID[30]  = "R3_1_7"; 
  m_DetNameFromDetID[31]  = "R4_1_7";
  m_DetNameFromDetID[32]  = "L5_5_5"; // top row 3
  m_DetNameFromDetID[33]  = "L4_6_7";
  m_DetNameFromDetID[34]  = "L3_6_7";
  m_DetNameFromDetID[35]  = "L2_5_5";	
  m_DetNameFromDetID[36]  = "C1_1_4";     
  m_DetNameFromDetID[37]  = "U1_1_3";
  m_DetNameFromDetID[38]  = "U1_2_3";
  m_DetNameFromDetID[39]  = "U1_3_3";
	
  m_DetNameFromDetID[40]  = "C1_2_4";    
  m_DetNameFromDetID[41]  = "R2_1_5";
  m_DetNameFromDetID[42]  = "R3_2_7";
  m_DetNameFromDetID[43]  = "R4_2_7";
  m_DetNameFromDetID[44]  = "R5_1_5";
  m_DetNameFromDetID[45]  = "L5_4_5"; //
  m_DetNameFromDetID[46]  = "L4_5_7";
  m_DetNameFromDetID[47]  = "L3_5_7";
  m_DetNameFromDetID[48]  = "L2_4_5";
  m_DetNameFromDetID[49]  = "L1_3_3";
    
  m_DetNameFromDetID[50]  = "R1_1_3";
  m_DetNameFromDetID[51]  = "R2_2_5";
  m_DetNameFromDetID[52]  = "R3_3_7";
  m_DetNameFromDetID[53]  = "R4_3_7";
  m_DetNameFromDetID[54]  = "R5_2_5";
  m_DetNameFromDetID[55]  = "L5_3_5";
  m_DetNameFromDetID[56]  = "L4_4_7";
  m_DetNameFromDetID[57]  = "L3_4_7";
  m_DetNameFromDetID[58]  = "L2_3_5";
  m_DetNameFromDetID[59]  = "L1_2_3";
    
  m_DetNameFromDetID[60]  = "R1_2_3";
  m_DetNameFromDetID[61]  = "R2_3_5";
  m_DetNameFromDetID[62]  = "R3_4_7";
  m_DetNameFromDetID[63]  = "R4_4_7";
  m_DetNameFromDetID[64]  = "R5_3_5";
  m_DetNameFromDetID[65]  = "L5_2_5";
  m_DetNameFromDetID[66]  = "L4_3_7";
  m_DetNameFromDetID[67]  = "L3_3_7";
  m_DetNameFromDetID[68]  = "L2_2_5";
  m_DetNameFromDetID[69]  = "L1_1_3";
    
  m_DetNameFromDetID[70]  = "R1_3_3";
  m_DetNameFromDetID[71]  = "R2_4_5";
  m_DetNameFromDetID[72]  = "R3_5_7";
  m_DetNameFromDetID[73]  = "R4_5_7";
  m_DetNameFromDetID[74]  = "R5_4_5";
  m_DetNameFromDetID[75]  = "L5_1_5";
  m_DetNameFromDetID[76]  = "L4_2_7";
  m_DetNameFromDetID[77]  = "L3_2_7";
  m_DetNameFromDetID[78]  = "L2_1_5";    
  m_DetNameFromDetID[79]  = "C1_4_4";
	
  m_DetNameFromDetID[80]  = "D1_3_3";    
  m_DetNameFromDetID[81]  = "D1_2_3";
  m_DetNameFromDetID[82]  = "D1_1_3";
  m_DetNameFromDetID[83]  = "C1_3_4";
  m_DetNameFromDetID[84]  = "R2_5_5";
  m_DetNameFromDetID[85]  = "R3_6_7";
  m_DetNameFromDetID[86]  = "R4_6_7";
  m_DetNameFromDetID[87]  = "R5_5_5";
  m_DetNameFromDetID[88]  = "L4_1_7";
  m_DetNameFromDetID[89]  = "L3_1_7";
    
  m_DetNameFromDetID[90]  = "C2_4_4";
  m_DetNameFromDetID[91]  = "D2_5_5";
  m_DetNameFromDetID[92]  = "D2_4_5";
  m_DetNameFromDetID[93]  = "D2_3_5";
  m_DetNameFromDetID[94]  = "D2_2_5";
  m_DetNameFromDetID[95]  = "D2_1_5";
  m_DetNameFromDetID[96]  = "C2_3_4";
  m_DetNameFromDetID[97]  = "R3_7_7";
  m_DetNameFromDetID[98]  = "R4_7_7";
  m_DetNameFromDetID[99]  = "C3_4_4";
    
  m_DetNameFromDetID[100] = "D3_7_7";
  m_DetNameFromDetID[101] = "D3_6_7";
  m_DetNameFromDetID[102] = "D3_5_7";
  m_DetNameFromDetID[103] = "D3_4_7";
  m_DetNameFromDetID[104] = "D3_3_7";
  m_DetNameFromDetID[105] = "D3_2_7";
  m_DetNameFromDetID[106] = "D3_1_7";
  m_DetNameFromDetID[107] = "C3_3_4";
  m_DetNameFromDetID[108] = "D4_7_7";
  m_DetNameFromDetID[109] = "D4_6_7";
    
  m_DetNameFromDetID[110] = "D4_5_7";
  m_DetNameFromDetID[111] = "D4_4_7";
  m_DetNameFromDetID[112] = "D4_3_7";
  m_DetNameFromDetID[113] = "D4_2_7";
  m_DetNameFromDetID[114] = "D4_1_7";
  m_DetNameFromDetID[115] = "D5_5_5";
  m_DetNameFromDetID[116] = "D5_4_5";
  m_DetNameFromDetID[117] = "D5_3_5";
  m_DetNameFromDetID[118] = "D5_2_5";
  m_DetNameFromDetID[119] = "D5_1_5";
  
  // Mapping from name to ID
  for ( int i = 0; i < 120; i++ ) 
    m_DetIDFromDetName[m_DetNameFromDetID[i]] = i;

  // creating plastic dE map
  m_dENameFromdEID[0]   = "dE_R";
  m_dENameFromdEID[1]   = "dE_L";
  m_dENameFromdEID[2]   = "dE_U";
  m_dENameFromdEID[3]   = "dE_D";

  for ( int i = 0; i < 4; i++ ) 
    m_dEIDFromdEName[m_dENameFromdEID[i]] = i;

  // creating Tracker map
  m_TrNameFromTrID[0]   = "F_01"; //Fixed Column - Base facing the Beam
  m_TrNameFromTrID[1]   = "F_02";
  m_TrNameFromTrID[2]   = "F_03";
  m_TrNameFromTrID[3]   = "F_04";
  m_TrNameFromTrID[4]   = "F_05";
  m_TrNameFromTrID[5]   = "F_06";
  m_TrNameFromTrID[6]   = "F_07";
  
  m_TrNameFromTrID[7]   = "F_08"; //Fixed Column - Filpped
  m_TrNameFromTrID[8]   = "F_09";
  m_TrNameFromTrID[9]   = "F_10";
  m_TrNameFromTrID[10]   = "F_11";
  m_TrNameFromTrID[11]   = "F_12";
  m_TrNameFromTrID[12]   = "F_13";
  m_TrNameFromTrID[13]   = "F_14";
  
  m_TrNameFromTrID[14]   = "B_01"; //Moving Column - Base facing the Beam
  m_TrNameFromTrID[15]   = "B_02";
  m_TrNameFromTrID[16]   = "B_03";
  m_TrNameFromTrID[17]   = "B_04";
  m_TrNameFromTrID[18]   = "B_05";
  m_TrNameFromTrID[19]   = "B_06";
  m_TrNameFromTrID[20]   = "B_07";
  
  m_TrNameFromTrID[21]   = "B_08"; //Moving Column - Flipped
  m_TrNameFromTrID[22]   = "B_09";
  m_TrNameFromTrID[23]   = "B_10";
  m_TrNameFromTrID[24]   = "B_11";
  m_TrNameFromTrID[25]   = "B_12";
  m_TrNameFromTrID[26]   = "B_13";
  m_TrNameFromTrID[27]   = "B_14";

  for ( int i = 0; i < 28; i++ ) 
    m_TrIDFromTrName[m_TrNameFromTrID[i]] = i;
}


//////////////////////////////////////////////////
//   Get                                        //
//////////////////////////////////////////////////
G4String BT2017DetMap::GetDetNameFromDetID(G4int id)
{
  return m_DetNameFromDetID[id];
}

G4int BT2017DetMap::GetDetIDFromDetName(G4String name)
{
  return m_DetIDFromDetName[name];
}

G4String BT2017DetMap::GetdENameFromdEID(G4int id)
{
  return m_dENameFromdEID[id];
}

G4int BT2017DetMap::GetdEIDFromdEName(G4String name)
{
  return m_dEIDFromdEName[name];
} 

G4String BT2017DetMap::GetTrNameFromTrID(G4int id)
{
  return m_TrNameFromTrID[id];
}

G4int BT2017DetMap::GetTrIDFromTrName(G4String name)
{
  return m_TrIDFromTrName[name];
} 
