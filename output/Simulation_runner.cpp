#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <thread>
#include <algorithm>
#include <valarray>


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
#include <root/TLatex.h>


#define PERPENDICULAR 0
#define PARALLEL 1

using namespace std;
using namespace ROOT;

Double_t NaN = std::numeric_limits<double>::quiet_NaN();
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

const int CN = 14; //total number of layer modules
const int nth = 8; // number of thread
const int Tn = CN/2; //number of front facing modules
const Double_t Tb = 6; //tracker module base in cm
const Double_t Tt = 2; //tracker module thickness in cm


valarray<Long64_t> EC(Long64_t(0),nth);
array<TH2F*,nth> HmapXY,HmapRP,HmapGXY,HmapGRP;
array<TH1F*,nth> HRa, HPh;
array<array<TH1F*,CN>,nth> Hf, Hb;
array<array<array<TH1F*,CN>,CN>,nth> HetaF, HetaB, Hoff;
array<array<array<TH2F*,CN>,CN>,nth> HratioF, HratioB;

array<string,CN> nameF, nameB;

array<Double_t,nth> gunX, gunY, gunTh, gunPh;
array<array<Double_t,CN>,nth> F, B;
array<array<array<Double_t,CN>,CN>,nth> etaF, etaB;

array<TChain*,nth> t;

TRandom3 tr;

Double_t fw = 0.0;						//error in measurement when particle incidents at edge of one tracker
Double_t d_lyso = 66.0;				//distance of source from start of lysos
Double_t dF = d_lyso - 7.0;		//distance of source from start of front layer
Double_t dB = d_lyso - 5.0;		//distance of source from start of back layer

// configuration can be set to perpendicular or parallel
bool configuration = PERPENDICULAR;

bool plot_slices = !configuration && true;
bool plot_map = !configuration && true;
bool plot_signals = false;
bool plot_ratios = false;
bool plot_offsets = configuration && true;
bool plot_generator = true;

bool is_nan(Double_t x) { return isnan(x); }

void* SR_func(void* ptr)
{
	Long64_t* M = (Long64_t*) ptr;
	cout<<"Running Thread: "<<M[0]<<endl;
	for(auto i=M[1]; i<M[2]; i++)
	{
		t[M[0]]->GetEntry(i);

		if(plot_generator)
		{
			gunX[M[0]] = t[M[0]]->GetLeaf("position_X")->GetTypedValue<float>();
			gunY[M[0]] = t[M[0]]->GetLeaf("position_Y")->GetTypedValue<float>();
			gunTh[M[0]] = t[M[0]]->GetLeaf("theta_lab")->GetTypedValue<float>();
			gunPh[M[0]] = t[M[0]]->GetLeaf("phi_lab")->GetTypedValue<float>();
		}

		for(int j=0; j<CN; j++)
		{
			F[M[0]][j] = t[M[0]]->GetLeaf(nameF[j].data())->GetTypedValue<float>();
			B[M[0]][j] = t[M[0]]->GetLeaf(nameB[j].data())->GetTypedValue<float>();
			if(plot_signals)
			{
				Hf[M[0]][j]->Fill(F[M[0]][j]);
				Hb[M[0]][j]->Fill(B[M[0]][j]);
			}
		}

		Double_t xB=NaN, yF=NaN, zB=NaN, zF=NaN;
		int tF=0, tB=0, ep=0;
		array<int,2> Fiv, Biv;      //index values
		for(int q=0; q<CN/2; q++)
		{
			for(int r=CN/2; r<CN; r++)
			{
				if((F[M[0]][q] && F[M[0]][r]) && (!isnan(F[M[0]][q]) && !isnan(F[M[0]][r])))
				{
					etaF[M[0]][q][r] = (F[M[0]][q] - F[M[0]][r]) / (F[M[0]][q] + F[M[0]][r]);
					HetaF[M[0]][q][r]->Fill(etaF[M[0]][q][r]);
					if(plot_ratios) HratioF[M[0]][q][r]->Fill(F[M[0]][q],F[M[0]][r]);
					ep = r - q - Tn;
					zF = (1 + pow(-1,ep+1)*etaF[M[0]][q][r])*(Tt/2);
					yF = zF * (Tb/4) * (2*Tn - 4*q - 3 + (1 - etaF[M[0]][q][r])*pow(-1,ep)) / (dF + 1 + etaF[M[0]][q][r]);
					Fiv = {q,r};
					tF = 1;
				}
				else if((count_if(F[M[0]].begin(),F[M[0]].end(),is_nan)==(CN-1)) && (!isnan(F[M[0]][q]) || !isnan(F[M[0]][r])))
				{
					tF = 2;
				}
				if((B[M[0]][q] && B[M[0]][r]) && (!isnan(B[M[0]][q]) && !isnan(B[M[0]][r])))
				{
					etaB[M[0]][q][r] = (B[M[0]][q] - B[M[0]][r]) / (B[M[0]][q] + B[M[0]][r]);
					HetaB[M[0]][q][r]->Fill(etaB[M[0]][q][r]);
					if(plot_ratios) HratioB[M[0]][q][r]->Fill(B[M[0]][q],B[M[0]][r]);
					ep = r - q - Tn;
					zB = (1 + pow(-1,ep+1)*etaB[M[0]][q][r])*Tt/2;
					xB = zB * (Tb/4) * (2*Tn - 4*q - 3 + (1 - etaB[M[0]][q][r])*pow(-1,ep)) / (dB + 1 + etaB[M[0]][q][r]);
					Biv = {q,r};
					tB = 1;
				}
				else if((count_if(B[M[0]].begin(),B[M[0]].end(),is_nan)==(CN-1)) && (!isnan(B[M[0]][q]) || !isnan(B[M[0]][r])))
				{
					tB = 2;
				}
			}
		}


		if(tF && tB)
		{
			Double_t xl = (d_lyso/zB)*xB;
			Double_t yl = (d_lyso/zF)*yF;
			if(plot_map) HmapXY[M[0]]->Fill(xl,yl);
			if(plot_map) HmapRP[M[0]]->Fill(atan2(yl,xl),sqrt(xl*xl + yl*yl));
			if(plot_slices) HRa[M[0]]->Fill(sqrt(xl*xl + yl*yl));
			if(plot_slices) HPh[M[0]]->Fill(atan2(yl,xl));
		}

		if((tF == 1) && (tB == 1) && plot_offsets)
		{
			Hoff[M[0]][Fiv[0]][Fiv[1]]->Fill(etaF[M[0]][Fiv[0]][Fiv[1]] - etaB[M[0]][Biv[0]][Biv[1]]);
		}

		if(plot_generator)
		{
			Double_t tR = d_lyso/cos(gunTh[M[0]]);			//tZ = d_lyso
			Double_t tX = tR*sin(gunTh[M[0]])*cos(gunPh[M[0]]);
			Double_t tY = tR*sin(gunTh[M[0]])*sin(gunPh[M[0]]);
			HmapGXY[M[0]]->Fill(tX,tY);
			HmapGRP[M[0]]->Fill(atan2(tY,tX),sqrt(tX*tX + tY*tY));
		}
		EC[M[0]]++;
	}
	return 0;
}

void init_vars()
{
	for(int i=0; i<nth; i++)
	{
		if(plot_map) HmapXY[i] = new TH2F("HmapXY", "X-Y Map;X;Y",500,-21,21,500,-21,21);
		if(plot_map) HmapRP[i] = new TH2F("HmapRP", "R-#phi Map;#phi;R",500,-4,4,500,0,21);
		if(plot_generator) HmapGXY[i] = new TH2F("HmapGen", "Generator XY Map;X;Y",500,-21,21,500,-21,21);
		if(plot_generator) HmapGRP[i] = new TH2F("HmapGen", "Generator R-#phi Map;#phi;R",500,-4,4,500,0,21);
		if(plot_slices) HRa[i] = new TH1F("HRa", "Radius Distribution;R;Counts",3000,0,21);
		if(plot_slices) HPh[i] = new TH1F("HPh", "#phi Distribution;#phi;Counts",800,-4,4);
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
	for(int i=0; i<nth; i++) VT[i]->Run();
	while(1)
	{
		cout<<"Progress: "<<(EC.sum() * 100) / N<<"%"<<endl;
		sleep(5);
		if((EC.sum() * 100) / N >= 99) break;
	}
	for(int i=0; i<nth; i++) VT[i]->Join();
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
			ss<<path_name<<Particle<<Target<<"-"<<Energy<<"MeV_t"<<j<<"-"<<i<<".root";
			cout<<"Filename: "<<ss.str()<<endl;
			for(int k=0; k<nth; k++) t[k]->Add(ss.str().data());
		}
	}
	N = t[0]->GetEntries();
	cout<<"Number of events in the tree: "<<N<<endl;

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
		for(unsigned int i=0; i<ECs.size(); i++)
		{
			HratioFm[ECs[i][0]][ECs[i][1]]->Write();
			HratioFm[ECs[i][0]][ECs[i][1]]->Draw("COLZ");
			cRatios->SaveAs((string("Ratios_Front.pdf") + string(i==0 ? "(" : "") + string(i==(ECs.size()-1) ? ")" : "")).data(),"pdf");
		}

		ECs.clear();
		for(int i=0; i<CN; i++) for(int j=0; j<CN; j++) if(HratioBm[i][j]->GetEntries()>0) ECs.push_back({i,j});
		for(unsigned int i=0; i<ECs.size(); i++)
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
		TH2F* HmapXYm = merge(HmapXY);
		TCanvas* cMap = new TCanvas("cMap", "X-Y Distribution", 1000, 1000);
		HmapXYm->Draw("COLZ");
		cMap->SaveAs("XYmap.pdf","pdf");
		HmapXYm->SaveAs("HmapXY.root","root");

		TH2F* HmapRPm = merge(HmapRP);
		TCanvas* cMap_2 = new TCanvas("cMap_2", "R-Phi Distribution", 1000, 1000);
		HmapRPm->Draw("COLZ");
		cMap_2->SaveAs("RPmap.pdf","pdf");
		HmapRPm->SaveAs("HmapRP.root","root");


		if(plot_slices)
		{
			TH1F* HRam = merge(HRa),* HPhm = merge(HPh);
			TCanvas* cSlicePh = new TCanvas("cSlicePh", "Phi Distribution", 1000, 1000);
			HPhm->Draw();
			cSlicePh->SaveAs("Slices.pdf(","pdf");
			HPhm->SaveAs("HPh.root","root");
			TCanvas* cSliceR = new TCanvas("cSliceR", "R Distribution", 1000, 1000);
			HRam->Draw();
			cSliceR->SaveAs("Slices.pdf)","pdf");
			HRam->SaveAs("HRa.root","root");
		}
	}
	if(plot_generator)
	{
		TH2F* HmapGXYm = merge(HmapGXY);
		TCanvas* cMap_3 = new TCanvas("cMap_3", "Generator X-Y Distribution", 1000, 1000);
		HmapGXYm->Draw("COLZ");
		cMap_3->SaveAs("GenXYmap.pdf","pdf");
		HmapGXYm->SaveAs("HmapGXY.root","root");

		TH2F* HmapGRPm = merge(HmapGRP);
		TCanvas* cMap_4 = new TCanvas("cMap_4", "Generator R-Phi Distribution", 1000, 1000);
		HmapGRPm->Draw("COLZ");
		cMap_4->SaveAs("GenRPmap.pdf","pdf");
		HmapGRPm->SaveAs("HmapGRP.root","root");
	}
	if(plot_offsets)
	{
		array<array<TH1F*,CN>,CN> Hoffm = merge(Hoff);
		TFile f("Offsets.root","RECREATE");
		TCanvas* cEta = new TCanvas("cEta","Eta Positions",1900,1000);
		cEta->Divide(3,1);

		vector<array<int,2>> ECs;
		for(int i=0; i<CN; i++) for(int j=0; j<CN; j++) if(Hoffm[i][j]->GetEntries()>0) ECs.push_back({i,j});
		for(unsigned int i=0; i<ECs.size(); i++)
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
	}
}


