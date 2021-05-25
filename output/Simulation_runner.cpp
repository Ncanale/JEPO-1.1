#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <thread>
#include <algorithm>
#include <valarray>

/*
//comment this section if ROOT is installed from source
#include <root/TCanvas.h>
#include <root/TChain.h>
#include <root/TH1.h>
#include <root/TLeaf.h>
#include <root/TH2.h>
#include <root/TThread.h>
#include <root/TROOT.h>
#include <root/TRandom3.h>
#include <root/TFile.h>
#include <root/TError.h>
*/

#define PERPENDICULAR 0
#define PARALLEL 1

using namespace std;
using namespace ROOT;

int n_runs = 5;
int n_steps = 7;
Long64_t N;
string path_name = "";

int step_id = 0;
int run_id = 0;

string Particle = "deuteron";
string Target = "Empty";
int Energy = 270;

int Dtheta = 8;
int Ds = 5;

const int CN = 14;
const int nth = 8;

valarray<Long64_t> EC(Long64_t(0),nth);
array<TH2F*,nth> Hmap;
array<TH1F*,nth> HRa, HTh;
array<array<TH1F*,CN>,nth> Hf, Hb;
array<array<array<TH1F*,CN>,CN>,nth> HetaF, HetaB, Hoff;
array<array<array<TH2F*,CN>,CN>,nth> HratioF, HratioB;

array<string,CN> nameF, nameB;

array<array<Double_t,CN>,nth> F, B;
array<array<array<Double_t,CN>,CN>,nth> etaF, etaB;

array<TChain*,nth> t;

TRandom3 tr;

Double_t fw = 0.1;
Double_t d_lyso = 66.0;
Double_t l1 = d_lyso - 7.0;
Double_t l2 = d_lyso - 5.0;

// configuration can be set to perpendicular or parallel
bool configuration = PERPENDICULAR;

bool plot_slices = !configuration && true;
bool plot_map = !configuration && true;
bool plot_signals = false;
bool plot_ratios = false;
bool plot_offsets = configuration && true;

bool is_nan(Double_t x) { return isnan(x); }

void* SR_func (void* ptr)
{
  Long64_t* N = (Long64_t*) ptr;
  cout<<"Running Thread: "<<N[0]<<endl;
  for(auto i=N[1]; i<N[2]; i++)
  {
    t[N[0]]->GetEntry(i);
    
    for(int j=0; j<CN; j++)
    {
      F[N[0]][j] = t[N[0]]->GetLeaf(nameF[j].data())->GetTypedValue<float>();
      B[N[0]][j] = t[N[0]]->GetLeaf(nameB[j].data())->GetTypedValue<float>();
      
      //cout<<N[0]<<": "<<F[N[0]][j]<<", "<<B[N[0]][j]<<", "<<(F[N[0]][j] && B[N[0]][j])<<endl;
      
      if(plot_signals)
      {
        Hf[N[0]][j]->Fill(F[N[0]][j]);
        Hb[N[0]][j]->Fill(B[N[0]][j]);
      }
      
      //F[N[0]][j] = (isnan(F[N[0]][j]) ? 0 : F[N[0]][j]);
      //B[N[0]][j] = (isnan(B[N[0]][j]) ? 0 : B[N[0]][j]);
    }
    
    Double_t xB=std::numeric_limits<double>::quiet_NaN(), yF=std::numeric_limits<double>::quiet_NaN();
    int tF=0, tB=0;
    array<int,2> Fiv, Biv;      //index values
    for(int q=0; q<CN/2; q++)
    {
      for(int r=CN/2; r<CN; r++)
      {
        //cout<<N[0]<<": CN/2 = "<<CN/2<<endl;
        if((F[N[0]][q] && F[N[0]][r]) && (!isnan(F[N[0]][q]) && !isnan(F[N[0]][r])))
        {
          etaF[N[0]][q][r] = (F[N[0]][q] - F[N[0]][r]) / (F[N[0]][q] + F[N[0]][r]);
          //cout<<N[0]<<": q = "<<q<<"; r = "<<r<<"; etaF = "<<etaF[N[0]][q][r]<<endl;
          HetaF[N[0]][q][r]->Fill(etaF[N[0]][q][r]);
          if(plot_ratios) HratioF[N[0]][q][r]->Fill(F[N[0]][q],F[N[0]][r]);
          yF = 6*(3-q) + 3*(1-etaF[N[0]][q][r])*(7.5+q-r) - 1.5;
          Fiv = {q,r};
          tF = 1;
        }
        else if((count_if(F[N[0]].begin(),F[N[0]].end(),is_nan)==(CN-1)) && (!isnan(F[N[0]][q]) || !isnan(F[N[0]][r])))
        {
          //cout<<N[0]<<": q = "<<q<<"; r = "<<r<<"; Edge on F"<<endl;
          if(!isnan(F[N[0]][q])) yF = 6*(3-q) + ((tr.Rndm()*2)-1)*fw - 1.5;
          if(!isnan(F[N[0]][r])) yF = 6*(10.5-r) + ((tr.Rndm()*2)-1)*fw - 1.5;
          tF = 2;
        }
        if((B[N[0]][q] && B[N[0]][r]) && (!isnan(B[N[0]][q]) && !isnan(B[N[0]][r])))
        {
          //cout<<N[0]<<": q = "<<q<<"; r = "<<r<<"; NO etaB yet!"<<endl;
          etaB[N[0]][q][r] = (B[N[0]][q] - B[N[0]][r]) / (B[N[0]][q] + B[N[0]][r]);
          //cout<<N[0]<<": q = "<<q<<"; r = "<<r<<"; etaB = "<<etaB[N[0]][q][r]<<endl;
          HetaB[N[0]][q][r]->Fill(etaB[N[0]][q][r]);
          //if(plot_ratios && F[N[0]][0]>5 && F[N[0]][0]<10 && F[N[0]][2]>8 && F[N[0]][2]<13) HratioB[N[0]][q][r]->Fill(B[N[0]][q],B[N[0]][r]);
          if(plot_ratios) HratioB[N[0]][q][r]->Fill(B[N[0]][q],B[N[0]][r]);
          xB = 6*(3-q) + 3*(1-etaB[N[0]][q][r])*(7.5+q-r) - 1.5;
          Biv = {q,r};
          tB = 1;
        }
        else if((count_if(B[N[0]].begin(),B[N[0]].end(),is_nan)==(CN-1)) && (!isnan(B[N[0]][q]) || !isnan(B[N[0]][r])))
        {
          //cout<<N[0]<<": q = "<<q<<"; r = "<<r<<"; Edge on B"<<endl;
          if(!isnan(B[N[0]][q])) xB = 6*(3-q) + ((tr.Rndm()*2)-1)*fw - 1.5;
          if(!isnan(B[N[0]][r])) xB = 6*(10.5-r) + ((tr.Rndm()*2)-1)*fw - 1.5;
          tB = 2;
        }
      }
    }
    
    if(tF && tB)
    {
      Double_t xl = (d_lyso/l2)*xB;
      Double_t yl = (d_lyso/l1)*yF;
      if(plot_map) Hmap[N[0]]->Fill(xl,yl);
      if(plot_slices) HRa[N[0]]->Fill(sqrt(xl*xl + yl*yl));
      if(plot_slices) HTh[N[0]]->Fill(atan2(yl,xl));
    }
    
		//if (tB == 1) cout<<tF<<", "<<tB<<endl;
    if ((tF == 1) && (tB == 1) && plot_offsets)
    {
			//cout<<Fiv[0]<<", "<<Fiv[1]<<", "<<Biv[0]<<", "<<Biv[1]<<endl;
      Hoff[N[0]][Fiv[0]][Fiv[1]]->Fill(etaF[N[0]][Fiv[0]][Fiv[1]] - etaB[N[0]][Biv[0]][Biv[1]]);
    }
    
    //cout<<N[0]<<": CN/2 = "<<CN/2<<endl;
    //if(i%10000==0) cout<<F[N[0]][0]<<", "<<B[N[0]][0]<<", "<<(F[N[0]][0] && B[N[0]][0])<<endl;
    EC[N[0]]++;
  }
  return 0;
}

void init_vars()
{
  for(int i=0; i<nth; i++)
  {
    if(plot_map) Hmap[i] = new TH2F("Hmap", "X-Y Map;X;Y",500,-21,21,500,-21,21);
    
    if(plot_slices) HRa[i] = new TH1F("HRa", "Radius Distribution;R;Counts",300,0,21);
    if(plot_slices) HTh[i] = new TH1F("HTh", "#theta Distribution;#theta;Counts",300,-4,4);
  }
  for(int i=0; i<CN; i++)
  {
    string sn = (i<9 ? string("0") + to_string(i+1) : to_string(i+1));
    nameF[i] = (string("kE_F_") + sn);
    nameB[i] = (string("kE_B_") + sn);
    
    for(int j=0; j<nth; j++)
    {
      Hf[j][i] = new TH1F(nameF[i].data(),(nameF[i]+string(";Energy;Counts")).data(),100,0,100);
      Hb[j][i] = new TH1F(nameB[i].data(),(nameB[i]+string(";Energy;Counts")).data(),100,0,100);
      for(int k=0; k<CN; k++)
      {
        HetaF[j][i][k] = new TH1F((string("HetaF_")+to_string(i)+string("_")+to_string(k)).data(),"F",200,-1.1,1.1);
        HetaB[j][i][k] = new TH1F((string("HetaB_")+to_string(i)+string("_")+to_string(k)).data(),"B",200,-1.1,1.1);
        if(plot_offsets) Hoff[j][i][k] = new TH1F((string("Hoff_")+to_string(i)+string("_")+to_string(k)).data(),
                                 (string("Offset_")+to_string(i)+string("_")+to_string(k)+string(";Offset Value;Counts")).data(),180,-2.1,2.1);
        
        if(plot_ratios)
        {
          HratioF[j][i][k] = new TH2F((string("HF_")+to_string(i)+string("_")+to_string(k)).data(),
                                  (string("F_")+to_string(i)+string("_")+to_string(k)+string("_ratio;F_")+to_string(i)+string("; F_")+to_string(k)).data(),
                                  200,0.3,30,200,0.3,30);
          HratioB[j][i][k] = new TH2F((string("HB_")+to_string(i)+string("_")+to_string(k)).data(),
                                  (string("B_")+to_string(i)+string("_")+to_string(k)+string("_ratio;B_")+to_string(i)+string("; B_")+to_string(k)).data(),
                                  200,0.3,30,200,0.3,30);
        }
      }
    }
  }
}

void run_threads()
{
  array<TThread*,nth> VT;
  array<Long64_t[3],nth> par;
  for(int i=0; i<nth; i++)
  {
    par[i][0] = i;
    par[i][1] = i*(N/nth);
    par[i][2] = (i+1)*(N/nth);
    VT[i] = new TThread((string("T")+to_string(i)).data(), SR_func, (void*) par[i]);
  }
  for(int i=0;i<nth;i++) VT[i]->Run();
	while(1)
	{
		cout<<"Progress: "<<(EC.sum() * 100) / N<<"%"<<endl;
		sleep(5);
		if((EC.sum() * 100) / N >= 99) break;
	}
  for(int i=0;i<nth;i++) VT[i]->Join();
}

template <typename T> T merge(array<T,nth> A)
{
  T HA;
  HA = (T) A[0]->Clone();
  for(int j=1; j<nth; j++) HA->Add(A[j]);
  return HA;
}

template <typename T> array<T,CN> merge(array<array<T,CN>,nth> A)
{
  array<T,CN> HA;
  for(int i=0; i<CN; i++)
  {
    HA[i] = (T) A[0][i]->Clone();
    for(int j=1; j<nth; j++) HA[i]->Add(A[j][i]);
  }
  return HA;
}

template <typename T> array<array<T,CN>,CN> merge(array<array<array<T,CN>,CN>,nth> A)
{
  array<array<T,CN>,CN> HA;
  for(int i=0; i<CN; i++)
  {
    for(int j=0; j<CN; j++)
    {
      HA[j][i] = (T) A[0][j][i]->Clone();
      for(int k=1; k<nth; k++) HA[j][i]->Add(A[k][j][i]);
    }
  }
  return HA;
}

void Simulation_runner()
{  
	gErrorIgnoreLevel = kError;
  for(int i=0; i<nth; i++) t[i] = new TChain("hits");
  stringstream ss;
  for(int i=0; i<n_runs; i++)
  {
    for(int j=0; j<nth; j++)
    {
      ss.str("");
      //ss<<path_name<<Particle<<Target<<"-"<<Energy<<"MeV_t"<<j<<"-"<<run_id<<"-"<<i<<".root";
      ss<<path_name<<Particle<<Target<<"-"<<Energy<<"MeV_t"<<j<<"-"<<i<<".root";
      cout<<"Filename: "<<ss.str()<<endl;
      for(int k=0; k<nth; k++) t[k]->Add(ss.str().data());
    }
  }
  N = t[0]->GetEntries();
  cout<<"Number of events in the tree: "<<N<<endl;
  
  //TH1F h("h","h",100,0,100);
  
  init_vars();

  run_threads();
  
  if(plot_signals) array<TH1F*,CN> Hfm = merge(Hf), Hbm = merge(Hb);
  array<array<TH1F*,CN>,CN> HetaFm = merge(HetaF), HetaBm = merge(HetaB);
  if(plot_ratios)
  {
    array<array<TH2F*,CN>,CN> HratioFm = merge(HratioF), HratioBm = merge(HratioB);
    TCanvas* cRatios = new TCanvas("cRatios", "Ratios", 1000, 1000);
    TFile f("Ratios.root","RECREATE");
    
    vector<array<int,2>> ECs;
    for(int i=0; i<CN; i++) for(int j=0; j<CN; j++) if(HratioFm[i][j]->GetEntries()>0) ECs.push_back({i,j});
    for(int i=0; i<ECs.size(); i++)
    {
      HratioFm[ECs[i][0]][ECs[i][1]]->Write();
      HratioFm[ECs[i][0]][ECs[i][1]]->Draw("COLZ");
      cRatios->SaveAs((string("Ratios_Front.pdf") + string(i==0 ? "(" : "") + string(i==(ECs.size()-1) ? ")" : "")).data(),"pdf");
    }
    
    ECs.clear();
    for(int i=0; i<CN; i++) for(int j=0; j<CN; j++) if(HratioBm[i][j]->GetEntries()>0) ECs.push_back({i,j});
    for(int i=0; i<ECs.size(); i++)
    {
      HratioBm[ECs[i][0]][ECs[i][1]]->Write();
      HratioBm[ECs[i][0]][ECs[i][1]]->Draw("COLZ");
      cRatios->SaveAs((string("Ratios_Back.pdf") + string(i==0 ? "(" : "") + string(i==(ECs.size()-1) ? ")" : "")).data(),"pdf");
    }
    
    f.Close();
    HratioBm[0][2]->Draw("SURF1");
  }
  if(plot_map)
  {
    TH2F* Hmapm = merge(Hmap);
    TCanvas* cMap = new TCanvas("cMap", "X-Y Distribution", 1000, 1000);
    Hmapm->Draw("COLZ");
    cMap->SaveAs("XYmap.pdf","pdf");
    Hmapm->SaveAs("Hmap.root","root");
		if(plot_slices)
		{
			TH1F* HRam = merge(HRa),* HThm = merge(HTh);
			TCanvas* cSliceTh = new TCanvas("cSliceTh", "Theta Distribution", 1000, 1000);
			HThm->Draw();
			cSliceTh->SaveAs("Slices.pdf(","pdf");
			HThm->SaveAs("HTh.root","root");
			TCanvas* cSliceR = new TCanvas("cSliceR", "R Distribution", 1000, 1000);
			HRam->Draw();
			cSliceR->SaveAs("Slices.pdf)","pdf");
			HRam->SaveAs("HRa.root","root");
		}
	}
	if(plot_offsets)
  {
    array<array<TH1F*,CN>,CN> Hoffm = merge(Hoff);
    TFile f("Offsets.root","RECREATE");
		TCanvas* cEta = new TCanvas("cEta","Eta Positions",1900,1000);
		cEta->Divide(3,1);

		vector<array<int,2>> ECs;
    for(int i=0; i<CN; i++) for(int j=0; j<CN; j++) if(Hoffm[i][j]->GetEntries()>0) ECs.push_back({i,j});
    for(int i=0; i<ECs.size(); i++)
    {
			cEta->cd(1);
			HetaFm[ECs[i][0]][ECs[i][1]]->Draw();
			cEta->cd(2);
			HetaBm[ECs[i][0]][ECs[i][1]]->Draw();
			cEta->cd(3);
			Hoffm[ECs[i][0]][ECs[i][1]]->Draw();
			Hoffm[ECs[i][0]][ECs[i][1]]->Write();
      cEta->SaveAs((string("Eta_Positions.pdf") + string(i==0 ? "(" : "") + string(i==(ECs.size()-1) ? ")" : "")).data(),"pdf");
		}
    f.Close();
    //cEta->SaveAs("Eta_Positions.pdf","pdf");
  }
}

