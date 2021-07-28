import ROOT as rt
import numpy as np
n_runs = 7
spect_sigma = 1.5
spect_th    = 0.3

# xpeak_sim= [-1.085, 0.24500000000000005, -0.0816666666666666, -0.7583333333333333, -0.4316666666666667, -1.4116666666666666, 0.5483333333333333]
# ypeak_sim = [108771.0, 110624.0, 92844.0, 91665.0, 85419.0, 80419.0, 71689.0]

# matrix2 = list(zip(xpeak_sim, ypeak_sim))
# print("UnSort ", matrix2)
# mat_sort = sorted(matrix2, key=lambda x: x[0])
# print("Sorted ", mat_sort)
# input()

# y_sim_sorted = [80419,108771,91665,85419,92844,110624,71689]
ypeak_TB    = [0.004089111858736201, 0.01132391844284614, 0.018024870023480702, 0.019738905525656134, 0.01830219717921987, 0.014120613115354088, 0.005270570607197707]

Y_g4_hist   =  [0.011182481252409825, 0.015465249070850017, 0.014313540416987758, 0.01363980699133594, 0.014832011337123224, 0.016258428396792958, 0.00842314604328572]


# def peak_fitter(hist,n_peak,spect_sigma,spect_th):
#     hist.Draw()
#     spect = rt.TSpectrum((n_peak))
#     nfound = spect.Search(hist,spect_sigma,"goffnobackground",spect_th) #
#     xpeaks = spect.GetPositionX()
#     ypeaks = spect.GetPositionY()
#     # xpeaks = [-15.6, -11.6, -8.35, 8.05, 11.93, 15.36]
#     xpeak_py = []
#     ypeak_py = []
#     for j in range (nfound): #it creates a List from a root buffer
#         xpeak_py.append(xpeaks[j])
#         ypeak_py.append(ypeaks[j])
#     # print('xpeak :', xpeak_py)
#     # print('ypeak :', ypeak_py)
#     return (ypeak_py[n_peak-1])

h1   = rt.TH1F("h_eta_offset","offset_TB",200,-2,2)
h2   = rt.TH1F("Hoff_3_11","offset_G4",200,-2,2)
h2_run = []
h2_run_buffer = []
for i in range(len(ypeak_TB)): 
    h2_run.append(rt.TH1F("Hoff_3_11_"+str(i)+"","offset_G4",200,-2,2))
    h2_run_buffer.append(rt.TH1F("Hoff_3_11_"+str(i)+"_buff","offset_G4",200,-2,2))
hSum   = rt.TH1F("hSum","HSum_G4",200,-2,2)
h3   = rt.THStack("h3","Stacked Offsets")
# h3   = rt.TH1F("h3","offsets",200,-2,2)
# peak_fit        = []
# peak_fit_error  = []
# sigma_fit       = []
# sigma_fit_error = []
# fit_parameters  = []

f = rt.TFile.Open("TB_offset.root")
f2 = rt.TFile.Open("Offsets.root")

h1 = f.Get("h_eta_offset")
h2 = f2.Get("Hoff_3_11")
for i in range (n_runs):
    name = "Hoff_run"+str(i)+"_3_11"
    print(name)
    # h_2[i] = f2.Get("Hoff_run"+str(i)+"_3_11")
    h2_run[i] = f2.Get(name)
# h1=hist.Clone("h1")
c1 = rt.TCanvas("c1", "Offsets")
c1.Divide(2)
c1.cd(1)
h1.Draw()
h1.SetLineColor(rt.kRed)

# c1.cd(2)
# for i in range (n_runs):
#     h_2[i].Draw('same')
#     Y_g4_hist.append(peak_fitter(h_2[i],1,spect_sigma,spect_th))
# print('Y ',Y_g4_hist)

c1.cd(2)
h1_buffer = h1.Clone()
h1_buffer.SetDrawOption("E1")
h2_buffer = h2.Clone()
for i in range (n_runs): h2_run_buffer[i] = h2_run[i].Clone()

h1_entries = int(h1.GetEntries())
h1_bins = int(h1.GetNbinsX())
print ("h1 bins",h1_bins )
for i in range(0,h1_bins):
    num = h1.GetBinContent(i)   
    h1_buffer.SetBinContent(i,num)
    # if ( i > 170):
    #     print ('value at i = 90', num/h1_entries)

h2_entries = int(h2.GetEntries())
h2_bins = int(h2.GetNbinsX())
print ("h2 bins",h2_bins )
print('h1_entries: ', h1_entries,' -  h2_entries:', h2_entries)
for i in range(0,h2_bins):
    num = h2.GetBinContent(i)   
    h2_buffer.SetBinContent(i,num*(h1_entries/h2_entries))
    # if ( i > 170):
    #     print ('value at i = 90', num/h2_entries)

ratio=[]
for i in range(len(Y_g4_hist)):
    ratio.append((ypeak_TB[i]*h1_entries)/(Y_g4_hist[i]*h2_entries))
print ('ratios:', ratio)
# h2.Draw()

for j in range (n_runs):
    h_2_entries = int(h2_run[j].GetEntries())
    h_2_bins = int(h2_run[j].GetNbinsX())
    print ("h_2_[",j,"] bins",h_2_bins)
    for i in range(0,h_2_bins):
        num = h2_run[j].GetBinContent(i)   
        h2_run_buffer[j].SetBinContent(i,num*(ratio[j]))
        # if ( i > 170):
        #     print ('value at i = 90', num/h2_entries

h1.Draw('E1')
h1.SetLineColor(rt.kRed)
for i in range (n_runs): h2_run_buffer[i].Draw("same")

c2 = rt.TCanvas("c2", "Overlaps")
c2.cd()
h1_buffer.Draw('E1')
h1_buffer.SetLineColor(rt.kRed)
for i in range (n_runs): 
    h2_run_buffer[i].Draw("same")
    hSum.Add(h2_run_buffer[i])
hSum.Draw('same')
hSum.SetLineColor(rt.kGreen+2)
# h2_buffer.Draw('same')
# h2_buffer.SetLineColor(rt.kYellow)

c3 = rt.TCanvas("c3", "Residuals")
c3.cd()
rp = rt.TRatioPlot(hSum,h1_buffer)
c3.SetTicks(-1, 2)
rp.Draw()
funct = rt.TF1('pol1','pol1') 
rp.GetLowerRefGraph().GetYaxis().SetRangeUser(-1,2)
rp.GetLowerRefGraph().Fit(funct,'','',-1.5,0.6)
print('Line Fit',funct.GetParameter(0),'\u00B1 ', funct.GetParError(0))
c3.Update()


ratio_calib =[]
for rat in ratio:
    ratio_calib.append(rat*(h1_entries/h2_entries))
print ('ratios \"calibrated\"', ratio_calib)
input('press a key')