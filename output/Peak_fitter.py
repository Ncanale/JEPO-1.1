import ROOT as rt
import numpy as np
import math as m
import time


configuration = "PARALLEL"
setting='TB'
plot_offsets = True

if configuration == "PARALLEL":
    if setting == "G4":
        names = ['Hoff_3_11']
        path_names = ['Offsets.root']
    ## REAL
    elif setting == "TB":
        names = ['h_eta_offset']
        path_names = ['TB_offset.root']
    else:
        input ('CHECK SETTINGS')

    canvas_names = ['can']
    canvas_titles =['Offset']
else:
    input ("CHECK THE CONFIGURATIONS")

rebin_value = 1 
n_runs      = 7


Target= "Empty"
Smearing=0.225
Energy=270

sigmas = []
sigmas_errors = []
units  = []

def root_sum_square(vector_error,calibration):
    quad_sum = 0
    for error in vector_error:
        error_cal = calibration* error
        quad = error_cal * error_cal
        quad_sum += quad
        # print ('error ', error ,' quad sum', quad_sum)
    quadratic_sum = np.sqrt(quad_sum)
    # print ('sqrt quad ', quadratic_sum,' cm')
    return (quadratic_sum)

def peak_fitter(canvas,hist,rebin_value,n_runs):
    canvas.cd()
    hist.Draw()
    hist.Rebin(rebin_value)
    
    amplitude   = []
    mean        = []
    sigma       = [] 
    amplitude_error   = []
    mean_error        = []
    sigma_error       = [] 
    Gauss             = [rt.TF1]*n_runs
    # # # print(fit_funcs)
    fit_string =''
    calibration_parameter = 1.5
    for j in range(n_runs):   
        if (j==n_runs-1):
            fit_string+=' gaus('+ str(j*3) + ')'
        else:
            fit_string+=' gaus('+ str(j*3) + ') +'
        # print (fit_string)

    # print('\n fit_string = ', fit_string)
    total_fit_func = rt.TF1('total_fit_funct',fit_string,-1.7,0.8)
    
    TB_peak_dist = [-1.39,-1.06,-0.73,-0.4,-0.06,0.23,0.53]
    TB_Amplitudes = [334,974,1500,1625,1458,1050,468]

    for j in range(n_runs): 
        # total_fit_func.SetParameter((j*3),(hist.GetEntriess()/n_runs))
        total_fit_func.SetParameter((j*3),TB_Amplitudes[j]/(m.sqrt(2*m.pi)*0.1)) #REAL 
        # total_fit_func.SetParameter((j*3)+1, -1.5 + j*0.4) #G4 
        total_fit_func.SetParameter((j*3)+1, TB_peak_dist[j]) #REAL
        total_fit_func.SetParameter((j*3)+2, 0.1)
    hist.Fit(total_fit_func,'','',-1.7,0.8)
    print('CHI/NDF of GAUSSIAN FIT: ',(total_fit_func.GetChisquare()/total_fit_func.GetNDF()),', Prob ', total_fit_func.GetProb())
    for j in range(n_runs): 
        amplitude.append(total_fit_func.GetParameter((j*3))/hist.GetEntries())
        amplitude_error.append(total_fit_func.GetParError((j*3))/hist.GetEntries())
        mean.append(total_fit_func.GetParameter((j*3)+1))
        mean_error.append(total_fit_func.GetParError((j*3)+1))
        sigma.append(total_fit_func.GetParameter((j*3)+2))
        sigma_error.append(total_fit_func.GetParError((j*3)+2))
        func_name = 'funct_'+str(j)
        Gauss[j] = rt.TF1(func_name,'[0]*exp(-0.5*((x-[1])/[2])**2)',-1.7,0.8)
        Gauss[j].SetParameters(total_fit_func.GetParameter(j*3),mean[j],sigma[j])
        Gauss[j].SetLineColor(j+1)
        Gauss[j].Draw('same')
    canvas.SaveAs('Test.pdf','pdf')

    # # # Calibration # #
    sigma_calbirated= []
    sigma_calbirated_error= []
    calibration_edges =[0.24739106414386924, 28.93078785640231]
    # if setting == 'G4':     calibration_edges =[0.1, 29.711556917623575]
    # elif setting == 'TB':   calibration_edges =[0.1, 29.9]

    Dx  = (mean[-1] - mean[0])
    print('peak -1 ', mean[-1] ,' peak 0 ', mean[0],'; diff',Dx )
    distance = (calibration_edges[1]-calibration_edges[0])/1
    calib_parameter = distance/Dx
    print ('CALIBRATION PARAMETER = ', calib_parameter,' mm/offset ')

    for sigmas in sigma : 
        sigma_calbirated.append(sigmas*calib_parameter)
    for error in sigma_error :
        sigma_calbirated_error.append(error*calib_parameter)
    print ('amplitudes ', amplitude)
    print ('means \t', mean)
    print ('expected means \t', TB_peak_dist)
    print ('sigmas calibrated', sigma_calbirated , '\u00B1',sigma_calbirated_error )
    print ('AVERAGE sigma : ', np.mean(sigma_calbirated),'\u00B1',np.mean(sigma_calbirated_error),' mm')

    return(amplitude,amplitude_error,mean,mean_error,sigma,sigma_error,sigma_calbirated,sigma_calbirated_error,calib_parameter)

f_s = open ("Residuals_scan.txt","a")
for i in range (0,len(names)):  
    position_calibrated = [] 
    path_name = path_names[i]# +'.root'
    print (path_name)
    f = rt.TFile.Open(path_name)
    canvas = rt.TCanvas(canvas_names[i],canvas_titles[i],2880,1800)
    result = peak_fitter(canvas,f.Get(names[i]),rebin_value,n_runs)

    
    print('\n AVERAGE sigma :', np.mean(result[-3]), ' \u00B1 ', np.mean(result[-2]),' mm')  
    calibration_parameter = result[-1]
    for mean in result[2]: position_calibrated.append(mean*calibration_parameter)
    print('Fitted position :',position_calibrated)
    diff = [] 
    for j in range(len(position_calibrated)-1):
        diff.append(position_calibrated[j+1]-position_calibrated[j])
    print('peak dist ', diff ,' mm')
    # input('press a key REMEMBER TO COPY AMPLITUDE LINE ON OFFSET PLOTTER! ')
    # input('continue')
    f.Close()
    if plot_offsets:
        ypeak_TB    = [0.004089111858736201, 0.01132391844284614, 0.018024870023480702, 0.019738905525656134, 0.01830219717921987, 0.014120613115354088, 0.005270570607197707]
        print('amplitudes ', result[0])
        Y_g4_hist   = result[0]

        h1   = rt.TH1F()
        h2   = rt.TH1F()
        h2_run = [rt.TH1F()]*n_runs
        h2_run_buffer = [rt.TH1F()]*n_runs
        f_TB = rt.TFile.Open("TB_offset.root")
        f2 = rt.TFile.Open("Offsets.root")

        h1 = f_TB.Get("h_eta_offset")
        h2 = f2.Get("Hoff_3_11")
        # h2 = f.Get(names[i])
        for k in range(len(ypeak_TB)): 
            name = "Hoff_run"+str(k)+"_3_11"
            # h2_run.append(rt.TH1F("Hoff_run"+str(k)+"_3_11","offset_G4",200,-2,2))
            # h2_run_buffer.append(rt.TH1F("Hoff_run"+str(k)+"_3_11_buff","offset_G4",200,-2,2))
            h2_run[k] = f2.Get(name)
        hSum   = rt.TH1F("hSum","HSum_G4",200,-2,2)
        h3   = rt.THStack("h3","Stacked Offsets")

        c1 = rt.TCanvas("c1", "Offsets")
        c1.Divide(2)
        c1.cd(1)
        h1.Draw()
        h1.SetLineColor(rt.kRed)
        c1.cd(2)
        h2_buffer = h2.Clone()
        for k in range (n_runs): h2_run_buffer[k] = h2_run[k].Clone()
        h1_entries = int(h1.GetEntries())
        h2_entries = int(h2.GetEntries())
        h2_bins = int(h2.GetNbinsX())
        print('h1_entries: ', h1_entries,' -  h2_entries:', h2_entries)
        print ("h2 bins",h2_bins )
        for k in range(0,h2_bins):
            num = h2.GetBinContent(k)   
            h2_buffer.SetBinContent(k,num*(h1_entries/h2_entries))     
        ratio=[]
        for k in range(len(Y_g4_hist)):
            ratio.append((ypeak_TB[k]*h1_entries)/(Y_g4_hist[k]*h2_entries))
        print ('ratios:', ratio) 
        for k in range (n_runs):
            h_2_entries = int(h2_run[k].GetEntries())
            h_2_bins = int(h2_run[k].GetNbinsX())
            print ("h_2_[",j,"] bins",h_2_bins)
            for p in range(0,h_2_bins):
                num = h2_run[k].GetBinContent(p)   
                h2_run_buffer[k].SetBinContent(p,num*(ratio[k]))

        h1.Draw('E1')
        h1.SetLineColor(rt.kRed)
        for k in range (n_runs): h2_run_buffer[k].Draw("same")
        c2 = rt.TCanvas("c2", "Overlaps")
        c2.cd()
        h1.Draw('E1')
        h1.SetLineColor(rt.kRed)
        for k in range (n_runs): 
            h2_run_buffer[k].Draw("same")
            hSum.Add(h2_run_buffer[k])
        hSum.Draw('same')
        hSum.SetLineColor(rt.kGreen+2)

        c3 = rt.TCanvas("c3", "Residuals")
        c3.cd()
        rp = rt.TRatioPlot(hSum,h1)
        c3.SetTicks(-1, 2)
        rp.Draw()
        funct = rt.TF1('pol1','pol1') 
        rp.GetLowerRefGraph().GetYaxis().SetRangeUser(-1,2)
        rp.GetLowerRefGraph().Fit(funct,'','',-1.5,0.6)
        print('Line Fit',funct.GetParameter(0),'\u00B1 ', funct.GetParError(0),' - CHI ', funct.GetChisquare())
        c3.Update()
        c3.SaveAs('residual_plot_'+str(Smearing)+'.pdf','pdf')
        c3.SaveAs('residual_plot_'+str(Smearing)+'.root','root')
        string = 'Smearing: ' + str(Smearing) +', Par 0: ' + str(funct.GetParameter(0)) + ', Par1: ' + str(funct.GetParameter(1)) + ', EPar 0: ' + str(funct.GetParError(0)) +', EPar 1: ' + str(funct.GetParError(1)) + ', CHI: '+ str(funct.GetChisquare()) + ', NDF: ' + str(funct.GetNDF()) +'\n'
        print(string)

        # ratio_scale =[]
        # for rat in ratio:
        #     ratio_scale.append(rat*(h1_entries/h2_entries))
        # print ('ratios \"scaled\"', ratio_scale)
        f_s.write(string)
f_s.close()


input('press a key')
    


