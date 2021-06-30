import ROOT as rt
h1   = rt.TH1F("h_eta_offset","offset_TB",200,-2,2)
h2   = rt.TH1F("Hoff_3_11","offset_G4",200,-2,2)
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
# h1=hist.Clone("h1")
c1 = rt.TCanvas("c1", "Offsets")
c1.Divide(3)
c1.cd(1)
h1.Draw()
h1.SetLineColor(rt.kRed)
c1.cd(2)
h2.Draw()
c1.cd(3)
factor = 1

h1_buffer = h1.Clone()
h2_buffer = h2.Clone()

h1_entries = int(h1.GetEntries())
h1_bins = int(h1.GetNbinsX())
print ("h1 bins",h1_bins )
for i in range(0,h1_bins):
    num = h1.GetBinContent(i)   
    h1_buffer.SetBinContent(i,num/h1_entries)
    # if ( i > 170):
    #     print ('value at i = 90', num/h1_entries)


h2_entries = int(h2.GetEntries())
h2_bins = int(h2.GetNbinsX())
print ("h2 bins",h2_bins )
for i in range(0,h2_bins):
    num = h2.GetBinContent(i)   
    h2_buffer.SetBinContent(i,num/h2_entries)
    # if ( i > 170):
    #     print ('value at i = 90', num/h2_entries)

h1_buffer.Draw('C')
h1_buffer.SetLineColor(rt.kRed)
h2_buffer.Draw("same")

c2 = rt.TCanvas("c2", "Overlaps")
c2.cd()
h1_buffer.Draw('C')
h1_buffer.SetLineColor(rt.kRed)
h2_buffer.Draw("same")

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

input('press a key')