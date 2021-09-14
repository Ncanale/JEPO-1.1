using namespace std;
using namespace ROOT;

void beam_profile(Long64_t N=100)
{
	TFile* F = new TFile("TB_eta_F.root");
	TH1 *h = new TH1F();//, *hC = new TH1F();
	F->GetObject("h_eta_F;1",h);
	fstream f;
	f.open("beam.in",fstream::out | fstream::trunc);
	
	for(Long64_t i=0; i<N; i++) f<<std::setprecision(8)<<gRandom->Gaus(0,1.0)<<"\t"<<(h->GetRandom() - 1) * 15<<"\t"<<Double_t(0)<<endl; 
	f.close();
}
