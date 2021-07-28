#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <thread>
#include <algorithm>
#include <valarray>
#include <list>
#include <tuple>


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

const int CN = 14; 										// total number of layer modules
const int n_runs = 15;								// number of runs -> radii, translations, etc
const int nth = 8; 										// number of threads (also to expect from file names)
const unsigned int TOP_N = 1; 				// the top N offset/ratio plots (based on no. of entries at tracker interface) to plot and store
const int Tn = CN/2; 									// number of front facing modules
const Double_t Tb = 6; 								// tracker module base in cm
const Double_t Tt = 2; 								// tracker module thickness in cm


valarray<Long64_t> EC(Long64_t(0),nth);
array<Long64_t,n_runs> NR;
array<TH2F*,nth> HmapXY,HmapRP,HmapThP,HmapGXY,HmapGRP;
array<TH1F*,nth> HRa, HPh, HTh;
array<array<TH1F*,n_runs>,nth> HRaR, HPhR, HThR;
array<array<TH1F*,CN>,nth> Hf, Hb;
array<array<array<TH1F*,CN>,CN>,nth> HetaF, HetaB, Hoff;
array<array<array<array<TH1F*,CN>,CN>,n_runs>,nth> HoffR;
array<array<array<TH2F*,CN>,CN>,nth> HratioF, HratioB;

array<string,CN> nameF, nameB;

array<Double_t,nth> gunX, gunY, gunTh, gunPh;
array<array<Double_t,CN>,nth> F, B;
array<array<array<Double_t,CN>,CN>,nth> etaF, etaB;

array<TChain*,nth> t;
array<array<TChain*,n_runs>,nth> trn;

TRandom3 tr;

Double_t fw = 0.0;						//error in measurement when particle incidents at edge of one tracker
Double_t d_lyso = 66.0;				//distance of source from start of lysos
Double_t dF = d_lyso - 7.0;		//distance of source from start of front layer
Double_t dB = d_lyso - 5.0;		//distance of source from start of back layer

// configuration can be set to perpendicular or parallel
bool configuration = PARALLEL;

bool plot_slices = !configuration && true;
bool plot_map = !configuration && true;
bool plot_signals = false;
bool plot_ratios = true;
bool plot_offsets = configuration && true;
bool plot_generator = true;

bool is_nan(Double_t x) { return isnan(x); }

void* SR_func(void* ptr)
{
	Long64_t* M = (Long64_t*) ptr;
	cout<<"Running Thread: "<<M[0]<<endl;
	for(auto i=M[1]; i<M[2]; i++)
	{
		trn[M[0]][M[3]]->GetEntry(i);

		if(plot_generator)
		{
			gunX[M[0]] = trn[M[0]][M[3]]->GetLeaf("position_X")->GetTypedValue<float>();
			gunY[M[0]] = trn[M[0]][M[3]]->GetLeaf("position_Y")->GetTypedValue<float>();
			gunTh[M[0]] = trn[M[0]][M[3]]->GetLeaf("theta_lab")->GetTypedValue<float>();
			gunPh[M[0]] = trn[M[0]][M[3]]->GetLeaf("phi_lab")->GetTypedValue<float>();
		}

		for(int j=0; j<CN; j++)
		{
			F[M[0]][j] = trn[M[0]][M[3]]->GetLeaf(nameF[j].data())->GetTypedValue<float>();
			B[M[0]][j] = trn[M[0]][M[3]]->GetLeaf(nameB[j].data())->GetTypedValue<float>();
			if(plot_signals)
			{
				Hf[M[0]][j]->Fill(F[M[0]][j]);
				Hb[M[0]][j]->Fill(B[M[0]][j]);
			}
		}

		Double_t xB=NaN, yF=NaN, zB=NaN, zF=NaN, tanThF = NaN, tanThB = NaN;
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
					tanThF = (Tb/4) * (2*Tn - 4*q - 3 + (1 - etaF[M[0]][q][r])*pow(-1,ep)) / (dF + 1 + etaF[M[0]][q][r]);
					yF = zF * tanThF;
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
					tanThB = (Tb/4) * (2*Tn - 4*q - 3 + (1 - etaB[M[0]][q][r])*pow(-1,ep)) / (dB + 1 + etaB[M[0]][q][r]);
					xB = zB * tanThB;
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
			Double_t xl = (dF/zB)*xB; //project at Front layer distance
			Double_t yl = (dF/zF)*yF;
			if(plot_map) HmapXY[M[0]]->Fill(xl,yl);
			if(plot_map) HmapRP[M[0]]->Fill(atan2(yl,xl),sqrt(xl*xl + yl*yl));
			if(plot_map) HmapThP[M[0]]->Fill(atan2(yl,xl),sqrt(xl*xl + yl*yl)/dF);

			if(plot_slices) HRa[M[0]]->Fill(sqrt(xl*xl + yl*yl));
			if(plot_slices) HPh[M[0]]->Fill(atan2(yl,xl));
			if(plot_slices) HTh[M[0]]->Fill(sqrt(xl*xl + yl*yl));

			if(plot_slices) HRaR[M[0]][M[3]]->Fill(sqrt(xl*xl + yl*yl));
			if(plot_slices) HPhR[M[0]][M[3]]->Fill(atan2(yl,xl));
			if(plot_slices) HThR[M[0]][M[3]]->Fill(sqrt(xl*xl + yl*yl)/dF);
		}

		if((tF == 1) && (tB == 1) && plot_offsets)
		{
			Hoff[M[0]][Fiv[0]][Fiv[1]]->Fill(etaF[M[0]][Fiv[0]][Fiv[1]] - etaB[M[0]][Biv[0]][Biv[1]]);
			HoffR[M[0]][M[3]][Fiv[0]][Fiv[1]]->Fill(etaF[M[0]][Fiv[0]][Fiv[1]] - etaB[M[0]][Biv[0]][Biv[1]]);
		}

		if(plot_generator)
		{
			Double_t tR = dF/cos(gunTh[M[0]]);			//tZ = d_lyso
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
		if(plot_map) HmapThP[i] = new TH2F("HmapThP", "#theta-#phi Map;#phi;#theta",500,-4,4,510,-0.01,0.5);
		if(plot_generator) HmapGXY[i] = new TH2F("HmapGen", "Generator XY Map;X;Y",500,-21,21,500,-21,21);
		if(plot_generator) HmapGRP[i] = new TH2F("HmapGen", "Generator R-#phi Map;#phi;R",500,-4,4,500,0,21);
		if(plot_slices) HRa[i] = new TH1F("HRa", "Radius Distribution;R;Counts",3000,0,21);
		if(plot_slices) HPh[i] = new TH1F("HPh", "#phi Distribution;#phi;Counts",3000,-4,4);
		if(plot_slices) HTh[i] = new TH1F("HTh", "#theta Distribution;#theta;Counts",510,-0.01,0.5);
		for(int j=0; j<n_runs; j++)
		{
			if(plot_slices) HRaR[i][j] = new TH1F((string("HRaR_")+to_string(j)).data(), "Radius Distribution per Run;R;Counts",3000,0,21);
			if(plot_slices) HPhR[i][j] = new TH1F((string("HPhR_")+to_string(j)).data(), "#phi Distribution per Run;#phi;Counts",3000,-4,4);
			if(plot_slices) HThR[i][j] = new TH1F((string("HThR_")+to_string(j)).data(), "#theta Distribution per Run;#phi;Counts",510,-0.01,0.5);
		}
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
				if(plot_offsets)
				{
					Hoff[j][i][k] = new TH1F((string("Hoff_")+to_string(i)+string("_")+to_string(k)).data(),
					        (string("Offset_")+to_string(i)+string("_")+to_string(k)+string(";Offset Value;Counts")).data(),200,-2,2);
					for(int l=0; l<n_runs; l++)
					{
						HoffR[j][l][i][k] = new TH1F((string("Hoff_run")+to_string(l)+string("_")+to_string(i)+string("_")+to_string(k)).data(),
										(string("Offset_")+to_string(i)+string("_")+to_string(k)+string(";Offset Value;Counts")).data(),200,-2,2);
					}
				}

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
	array<Long64_t[4],nth> par;
	for(int j=0; j<n_runs; j++)
	{
		cout<<"For Run "<<j<<":"<<endl;
		for(int i=0; i<nth; i++)
		{
			par[i][0] = i;
			par[i][1] = i*(NR[j]/nth);
			par[i][2] = (i+1)*(NR[j]/nth);
			par[i][3] = j;
			VT[i] = new TThread((string("T")+to_string(i)).data(), SR_func, (void*) par[i]);
		}
		for(int i=0; i<nth; i++)
		{
			VT[i]->Run();
			usleep(10000);
		}
		while(1)
		{
			cout<<"Progress: "<<(EC.sum() * 100) / N<<"%"<<endl;
			sleep(5);
			if((EC.sum() * 100) / N >= ((j+1)*99/n_runs)) break;
		}
		for(int i=0; i<nth; i++) VT[i]->Join();
	}
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
template <typename T> array<T,n_runs> merge(array<array<T,n_runs>,nth> A)
{
	array<T,n_runs> HA;
	for(int i=0; i<n_runs; i++)
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

template <typename T> array<array<array<T,CN>,CN>,n_runs> merge(array<array<array<array<T,CN>,CN>,n_runs>,nth> A)
{
	array<array<array<T,CN>,CN>,n_runs> HA;
	for(int i=0; i<CN; i++)
	{
		for(int j=0; j<CN; j++)
		{
			for(int k=0; k<n_runs; k++)
			{
				HA[k][j][i] = (T) A[0][k][j][i]->Clone();
				for(int l=1; l<nth; l++) HA[k][j][i]->Add(A[l][k][j][i]);
			}
		}
	}
	return HA;
}

bool sorting(tuple<int,int,Long64_t> AA, tuple<int,int,Long64_t> BB) { return get<2>(AA) > get<2>(BB); }

void Simulation_runner()
{
	gErrorIgnoreLevel = kError;
	for(int i=0; i<nth; i++)
	{
		t[i] = new TChain("hits");
		for(int j=0; j<n_runs; j++) trn[i][j] = new TChain("hits");
	}
	stringstream ss;
	for(int i=0; i<n_runs; i++)
	{
		for(int j=0; j<nth; j++)
		{
			ss.str("");
			ss<<path_name<<Particle<<Target<<"-"<<Energy<<"MeV_t"<<j<<"-"<<i<<".root";
			cout<<"Filename: "<<ss.str()<<endl;
			for(int k=0; k<nth; k++)
			{
				t[k]->Add(ss.str().data());
				trn[k][i]->Add(ss.str().data());
			}
		}
	}
	N = t[0]->GetEntries();
	for(int i=0; i<n_runs; i++) NR[i] = trn[0][i]->GetEntries();
	cout<<"Number of events in the tree: "<<N<<endl;

	init_vars();

	run_threads();

	if(plot_signals) array<TH1F*,CN> Hfm = merge(Hf), Hbm = merge(Hb);
	array<array<TH1F*,CN>,CN> HetaFm = merge(HetaF), HetaBm = merge(HetaB);
	if(plot_ratios)
	{
		array<array<TH2F*,CN>,CN> HratioFm = merge(HratioF), HratioBm = merge(HratioB);
		TCanvas* cRatios = new TCanvas("cRatios", "Ratios", 1000, 1000);
		cRatios->Divide(2,1);
		TFile f("Ratios.root","RECREATE");

		list<tuple<int,int,Long64_t>> ECsF, ECsB;
		for(int i=0; i<CN; i++) for(int j=0; j<CN; j++)
		{
			ECsF.push_back(make_tuple(i,j,HratioFm[i][j]->GetEntries()));
			ECsB.push_back(make_tuple(i,j,HratioBm[i][j]->GetEntries()));
		}
		ECsF.sort(sorting);
		ECsB.sort(sorting);
		auto itF = ECsF.begin(), itB = ECsB.begin();
		
		for(unsigned int i=0; i<TOP_N; i++)
		{
			int a,b;
			tie(a,b,ignore) = *itF;
			HratioFm[a][b]->Write();
			cRatios->cd(1);
			HratioFm[a][b]->Draw("COLZ");

			tie(a,b,ignore) = *itB;
			HratioBm[a][b]->Write();
			cRatios->cd(2);
			HratioBm[a][b]->Draw("COLZ");
			cRatios->SaveAs((string("Ratios.pdf") + string(i==0 && TOP_N!=1 ? "(" : "") + string(i==(TOP_N-1) && TOP_N!=1 ? ")" : "")).data(),"pdf");
			
			++itF;
			++itB;
		}

		/*
		ECs.clear();
		for(int i=0; i<CN; i++) for(int j=0; j<CN; j++) ECs.push_back(make_tuple(i,j,HratioBm[i][j]->GetEntries()));
		ECs.sort(sorting);
		it = ECs.begin();

		for(unsigned int i=0; i<TOP_N; i++)
		{
			int a,b;
			tie(a,b,ignore) = *it;
			HratioBm[a][b]->Write();
			HratioBm[a][b]->Draw("COLZ");
			cRatios->SaveAs((string("Ratios_Back.pdf") + string(i==0 && TOP_N!=1 ? "(" : "") + string(i==(TOP_N-1) && TOP_N!=1 ? ")" : "")).data(),"pdf");
		}
		*/
		f.Close();
		//HratioBm[0][2]->Draw("SURF1");
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
		
		TH2F* HmapThPm = merge(HmapThP);
		TCanvas* cMap_3 = new TCanvas("cMap_3", "Theta-Phi Distribution", 1000, 1000);
		HmapThPm->Draw("COLZ");
		cMap_3->SaveAs("ThPmap.pdf","pdf");
		HmapThPm->SaveAs("HmapThP.root","root");

		if(plot_slices)
		{
			TH1F* HRam = merge(HRa),* HPhm = merge(HPh), * HThm = merge (HTh);
			TCanvas* cSlicePh = new TCanvas("cSlicePh", "Phi Distribution", 1000, 1000);
			HPhm->Draw();
			cSlicePh->SaveAs("Slices.pdf(","pdf");
			HPhm->SaveAs("HPh.root","root");
			TCanvas* cSliceR = new TCanvas("cSliceR", "R Distribution", 1000, 1000);
			HRam->Draw();
			cSliceR->SaveAs("Slices.pdf","pdf");
			HRam->SaveAs("HRa.root","root");
			TCanvas* cSliceTh = new TCanvas("cSliceTh", "Theta Distribution", 1000, 1000);
			HThm->Draw();
			cSliceTh->SaveAs("Slices.pdf)","pdf");
			HThm->SaveAs("HTh.root","root");
	
			array<TH1F*,n_runs> HRaRm = merge(HRaR), HPhRm = merge(HPhR), HThRm = merge(HThR);
			TCanvas* cSlicePhR	= new TCanvas("cSlicePhR", "Phi Distribution per Run", 1000, 1000);
			TFile fPhR("HPhR.root","RECREATE");
			cSlicePhR->cd();
			for (int j=0;j<n_runs;j++)
			{
				HPhRm[j]->Draw();
				cSlicePhR->SaveAs((string("Phi_Slices_run.pdf") + string(j==0 ? "(" : "") + string(j==(n_runs-1) ? ")" : "")).data(),"pdf");
				HPhRm[j]->Write();
			}
			fPhR.Close();
			TCanvas* cSliceThR	= new TCanvas("cSliceThR", "Theta Distribution per Run", 1000, 1000);
			TFile fThR("HThR.root","RECREATE");
			cSliceThR->cd();
			for (int j=0;j<n_runs;j++)
			{
				HThRm[j]->Draw();
				cSliceThR->SaveAs((string("Theta_Slices_run.pdf") + string(j==0 ? "(" : "") + string(j==(n_runs-1) ? ")" : "")).data(),"pdf");
				HThRm[j]->Write();
			}
			fThR.Close();
			TCanvas* cSliceRR 	= new TCanvas("cSliceRR", "R Distribution per Run", 1000, 1000);
			TFile fRR("HRR.root","RECREATE");
			cSliceRR->cd();
			for (int j=0;j<n_runs;j++)
			{
				HRaRm[j]->Draw();
				cSliceRR->SaveAs((string("R_Slices_run.pdf") + string(j==0 ? "(" : "") + string(j==(n_runs-1) ? ")" : "")).data(),"pdf");
				HRaRm[j]->SaveAs("HRR.root","root");
			}
			fRR.Close();
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
		TFile f("Offsets.root","RECREATE");
		TCanvas* cOffR = new TCanvas("cOffR","Offsets for runs",1900,1000);
		array<array<TH1F*,CN>,CN> Hoffm = merge(Hoff);
		list<tuple<int,int,Long64_t>> ECs;
		for(int i=0; i<CN; i++) for(int j=0; j<CN; j++) ECs.push_back(make_tuple(i,j,Hoffm[i][j]->GetEntries()));
		ECs.sort(sorting);
		
		array<array<array<TH1F*,CN>,CN>,n_runs> HoffRm = merge(HoffR);
		auto it = ECs.begin();
		for(unsigned int j=0; j<TOP_N; j++)
		{
			for(int i=0; i<n_runs; i++)
			{
				int a,b;
				tie(a,b,ignore) = *it;
				HoffRm[i][a][b]->Draw();
				HoffRm[i][a][b]->Write();
				cOffR->SaveAs((string("Offsets_For_Run_") + to_string(i) + string(".pdf") + string(j==0 && TOP_N!=1 ? "(" : "") + string(j==(TOP_N-1) && TOP_N!=1 ? ")" : "")).data(),"pdf");
			}
			++it;
		}

		TCanvas* cEta = new TCanvas("cEta","Eta Positions",1900,1000);
		cEta->Divide(3,1);
		it = ECs.begin();
		for(unsigned int i=0; i<TOP_N; i++)
		{
			int a,b;
			tie(a,b,ignore) = *it;
			cEta->cd(1);
			HetaFm[a][b]->Draw();
			cEta->cd(2);
			HetaBm[a][b]->Draw();
			cEta->cd(3);
			Hoffm[a][b]->Draw();
			Hoffm[a][b]->Write();
			cEta->SaveAs((string("Eta_Positions.pdf") + string(i==0 && TOP_N!=1 ? "(" : "") + string(i==(TOP_N-1) && TOP_N!=1 ? ")" : "")).data(),"pdf");
			++it;
		}
		
		f.Close();
	}
}


