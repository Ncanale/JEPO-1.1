import ROOT as rt
import numpy as np
n_runs = 7

xpeak_sim= [-1.085, 0.24500000000000005, -0.0816666666666666, -0.7583333333333333, -0.4316666666666667, -1.4116666666666666, 0.5483333333333333]
ypeak_sim = [108771.0, 110624.0, 92844.0, 91665.0, 85419.0, 80419.0, 71689.0]

# matrix2 = list(zip(xpeak_sim, ypeak_sim))
# print("UnSort ", matrix2)
# mat_sort = sorted(matrix2, key=lambda x: x[0])
# print("Sorted ", mat_sort)
# input()

y_sim_sorted = [80419,108771,91665,85419,92844,110624,71689]
ypeak_TB = [284,944,1458,1635,1444,1125,497]
ratio=[]
for i in range(len(y_sim_sorted)):
    ratio.append(y_sim_sorted[i]/ypeak_TB[i]) 
print ('ratios:', ratio)


h1   = rt.TH1F("h_eta_offset","offset_TB",200,-2,2)
h2   = rt.TH1F("Hoff_3_11","offset_G4",200,-2,2)
h_2        = [rt.TH1F("Hoff_3_11","offset_G4",200,-2,2)]*n_runs
h_2_buffer  = [rt.TH1F("Hoff_3_11","offset_G4",200,-2,2)]*n_runs
h3   = rt.THStack("h3","Stacked Offsets")
# h3   = rt.TH1F("h3","offsets",200,-2,2)
peak_fit        = []
peak_fit_error  = []
sigma_fit       = []
sigma_fit_error = []
fit_parameters  = []

f = rt.TFile.Open("TB_offset.root")
f2 = rt.TFile.Open("Offsets.root")

h1 = f.Get("h_eta_offset")
h2 = f2.Get("Hoff_3_11")
for i in range (n_runs):
    name = "Hoff_run"+str(i)+"_3_11"
    print(name)
    # h_2[i] = f2.Get("Hoff_run"+str(i)+"_3_11")
    h_2[i] = f2.Get(name)
# h1=hist.Clone("h1")
c1 = rt.TCanvas("c1", "Offsets")
c1.Divide(3)
c1.cd(1)
h1.Draw()
h1.SetLineColor(rt.kRed)
c1.cd(2)
for i in range (n_runs):
    h_2[i].Draw('same')
# h2.Draw()
c1.cd(3)
factor = 1

h1_buffer = h1.Clone()
h2_buffer = h2.Clone()
for i in range (n_runs): h_2_buffer[i] = h_2[i].Clone()

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
for i in range(0,h2_bins):
    num = h2.GetBinContent(i)   
    h2_buffer.SetBinContent(i,num*(h1_entries/h2_entries))
    # if ( i > 170):
    #     print ('value at i = 90', num/h2_entries)

for j in range (n_runs):
    h_2_entries = int(h_2[j].GetEntries())
    h_2_bins = int(h_2[j].GetNbinsX())
    print ("h_2_[",j,"] bins",h_2_bins)
    for i in range(0,h_2_bins):
        num = h_2[j].GetBinContent(i)   
        h_2_buffer[j].SetBinContent(i,num/(ratio[j]))
        # if ( i > 170):
        #     print ('value at i = 90', num/h2_entries)

h1_buffer.Draw('L')
h1_buffer.SetLineColor(rt.kRed)
for i in range (n_runs): h_2_buffer[i].Draw("same")

c2 = rt.TCanvas("c2", "Overlaps")
c2.cd()
h1_buffer.Draw('')
h1_buffer.SetLineColor(rt.kRed)
for i in range (n_runs): h_2_buffer[i].Draw("same")
h2_buffer.Draw('same')
h2_buffer.SetLineColor(rt.kGreen)

# h1 = rt.TH1F("h1", "h1", 100, -5, 5)
# h1.FillRandom("gaus", 3000)
# funct = rt.TF1('funct','gaus') 
# h1.Fit(funct,"IQ0","",range[0],range[1])

# peak_fit.append(funct.GetParameter(1))
# peak_fit_error.append(funct.GetParError(1))
# sigma_fit.append(funct.GetParameter(2))
# sigma_fit_error.append(funct.GetParError(2))
# fit_parameters.append(funct.GetParameter(0))
# fit_parameters.append(funct.GetParameter(1))
# fit_parameters.append(funct.GetParameter(2))    
# # h1.GetXaxis().SetTitle("x")
# # c1.Clear() # Fit does not draw into correct pad
# # rp1 = rt.TRatioPlot(h1)
# # rp1.Draw()
# # rp1.GetLowerRefYaxis().SetTitle("ratio")
# # rp1.GetUpperRefYaxis().SetTitle("entries")
# c1.Update()



# # c2 = rt.TCanvas("21", "fit residual simple")
# # c2.cd()

# # h1[0]=hist.Clone("h1")
# # funct = rt.TF1('funct','gaus') 
# # h1[0].Fit(funct,"IQ0","",-1,1)
# # # h1.GetXaxis().SetTitle("x")
# # # c1.Clear() # Fit does not draw into correct pad
# # rp1 = rt.TRatioPlot(h1[0])
# # rp1.Draw()
# # # rp1.GetLowerRefYaxis().SetTitle("ratio")
# # # rp1.GetUpperRefYaxis().SetTitle("entries")
# # c2.Update()


# print ("Peak fit ", peak_fit,' error ', peak_fit_error)
ratio_calib =[]
for rat in ratio:
    ratio_calib.append(rat*(h1_entries/h2_entries))
print ('ratios \"calibrated\"', ratio_calib)
input('press a key')