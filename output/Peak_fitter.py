import ROOT as rt
import numpy as np
import math as m
import time


configuration = "PERPENDICULAR"
setting = "PHI"
plot_offsets = False
residuals = False

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
elif configuration == "PERPENDICULAR":
    if setting == "THETA":
        names = ['HTh']
        path_names = ['HTh.root']
        canvas_titles =['Theta scan']
    elif setting == "PHI":
        # names = ['HPh']
        # path_names = ['HPh.root']
        names = ['HPh_deg']
        path_names = ['HPh_deg.root']
        canvas_titles =['Phi scan']
    else:
        input ("CHECK SETTINGS!")
    canvas_names = ['can']

else:
    input ("CHECK THE CONFIGURATIONS")

rebin_value = 1 
n_runs      = 39


Target= "Empty"
Smearing=0.22
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

def find_the_values(hist_root_name):
    hist_entries = hist_root_name.GetEntries()
    hist_max = hist_root_name.GetMaximum()
    hist_mean = hist_root_name.GetMean()
    hist_std = hist_root_name.GetStdDev()
    return(hist_entries, hist_max, hist_mean, hist_std)

def peak_fitter(canvas1,canvas2,hist,rebin_value,n_runs,TB_peak_dist,TB_Y_max):
    canvas1.cd()
    hist.Draw()
    hist.Rebin(rebin_value)
    
    amplitude   = []
    mean        = []
    sigma       = [] 
    amplitude_error   = []
    mean_error        = []
    sigma_error       = [] 
    Gauss             = [rt.TF1]*n_runs
    Sigma_vs_Angle=rt.TGraphErrors(n_runs)
    Sigma_vs_Angle_mean=rt.TGraphErrors(2)
    phi_radiants      = []
    # # # print(fit_funcs)
    fit_string =''
    calibration_parameter = 1.5
        # print (fit_string)

    # print('\n fit_string = ', fit_string)
    # total_fit_func = rt.TF1('total_fit_funct',fit_string,-1.6,0.7)
    
    

    for j in range(n_runs):   
        if (j==n_runs-1):
            fit_string+=' gaus('+ str(j*3) + ')'
        else:
            fit_string+=' gaus('+ str(j*3) + ') +'
    if configuration == "PARALLEL":
        TB_peak_dist = [-1.39,-1.06,-0.73,-0.4,-0.06,0.23,0.53]
        TB_Y_max = [334,1100,1550,1625,1458,1050,468]
        fit_range = [-1.55,0.7]
    elif configuration == "PERPENDICULAR":
        if setting == "PHI":
            # phi_array= [0.0, 22.5, 45.0, 67.5, 90.0, 112.5, 135.0, 157.5, 202.5,  225.0, 247.5, 270.0, 292.5, 315.0, 337.5]
            # phi_radiants=[0.0, 0.39269908169872414, 0.7853981633974483, 1.1780972450961724, 1.5707963267948966, 1.9634954084936207, 2.356194490192345, 2.748893571891069, -2.748893571891069, -2.356194490192345, -1.9634954084936207, -1.5707963267948966, -1.1780972450961724, -0.7853981633974483, -0.39269908169872414]
            # radiants = [phi * np.pi/180 for phi in phi_array]
            # phi_radiants = np.arctan2(np.sin(radiants),np.cos(radiants))
            # phi_radiants.sort()
            # print ('PHI RAD', phi_radiants,' - ', len(phi_radiants))
            # phi_sigma = 0.005
            # fit_range = [min(phi_radiants)-0.3,max(phi_radiants)+0.3]

            phi_array= [5.625, 11.25, 22.5, 33.75, 45.0, 56.25, 67.5, 78.75, 84.375, 90.0, 95.625, 101.25, 112.5, 123.75, 135.0, 146.25, 157.5, 168.75, 174.375, 180.0, 185.625, 191.25, 202.5, 213.75, 225.0, 236.25, 247.5, 258.75, 264.375, 270.0, 275.625, 281.25, 292.5, 303.75, 315.0, 326.25, 337.5, 348.75, 354.375]
            phi_array = [phi - 180.0 for phi in phi_array]
            phi_sigma = 0.1
            fit_range = [min(phi_array)-3,max(phi_array)+3]
        
        elif setting == "THETA":
            # theta_array = []
            # theta_start = 5 
            # theta_step = 2
            # theta_step2 = theta_step * 1.06 
            # # theta_sigma = 3/n_runs
            # theta_sigma = 0.1
            # fit_range = [theta_start-1,theta_start + (theta_step2*n_runs)+1]

            theta_array=[5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 10.5, 11, 11.5, 12, 12.5, 13, 13.5, 14, 14.5, 15, 15.5, 16, 16.5, 17, 17.5] 
            theta_array.sort()
            theta_sigma = 0.1
            fit_range = [min(theta_array)-2,max(theta_array)+1]

    total_fit_func = rt.TF1('total_fit_funct',fit_string,fit_range[0],fit_range[1])
    total_fit_func.SetNpx(1000)

    for j in range(n_runs): 
        if configuration == "PARALLEL":
            total_fit_func.SetParameter((j*3),TB_Y_max[j]/(m.sqrt(2*m.pi)*0.1)) #REAL 
            print ('Set Parameter(',j*3,') \t: \t',TB_Y_max[j]/(m.sqrt(2*m.pi)*0.1))
            total_fit_func.SetParameter((j*3)+1, TB_peak_dist[j]) 
            print ('Set Parameter(',(j*3) +1,') \t: \t',TB_peak_dist[j])
            total_fit_func.SetParameter((j*3)+2, 0.1)
            print ('Set Parameter(',(j*3)+2,') \t: \t',0.1)
            fit_range = [-1.55,0.7]
        elif configuration == "PERPENDICULAR":
            if setting == "PHI":
                total_fit_func.SetParameter((j*3),hist.GetMaximum()*99/100)  
                print ('Set Parameter(',j*3,') \t: \t',hist.GetMaximum()*99/100)
                # total_fit_func.SetParameter((j*3)+1, phi_radiants[j] ) 
                # print ('Set Parameter(',(j*3) +1,') \t: \t', phi_radiants[j])
                total_fit_func.SetParameter((j*3)+1, phi_array[j] ) 
                print ('Set Parameter(',(j*3) +1,') \t: \t', phi_array[j])
                total_fit_func.SetParameter((j*3)+2,1)  
                print ('Set Parameter(',(j*3)+2,') \t: \t', phi_sigma)
                total_fit_func.SetParameter((j*3)+2, phi_sigma)
                # fit_range = [min(phi_radiants)-0.3,max(phi_radiants)+0.3]
            elif setting == "THETA":

                total_fit_func.SetParameter((j*3),hist.GetMaximum()*99/100)  
                print ('Set Parameter(',j*3,') \t: \t',hist.GetMaximum()*99/100)
                # total_fit_func.SetParameter((j*3)+1, phi_radiants[j] ) 
                # print ('Set Parameter(',(j*3) +1,') \t: \t', phi_radiants[j])
                total_fit_func.SetParameter((j*3)+1, theta_array[j] ) 
                print ('Set Parameter(',(j*3) +1,') \t: \t', theta_array[j])
                total_fit_func.SetParameter((j*3)+2,1)  
                print ('Set Parameter(',(j*3)+2,') \t: \t', theta_sigma)
                total_fit_func.SetParameter((j*3)+2, theta_sigma)

                # theta_array.append(theta_start + (theta_step2*j))
                # print ('Set Parameter(',j*3,') \t: \t',hist.GetMaximum()*90/100)
                # total_fit_func.SetParameter((j*3),hist.GetMaximum()*90/100)  
                # print ('Set Parameter(',(j*3) +1,') \t: \t', (theta_start + (theta_step2*j)))
                # total_fit_func.SetParameter((j*3)+1, theta_start  + (theta_step2*j) ) 
                # print ('Set Parameter(',(j*3) +2,') \t: \t',theta_sigma)
                # total_fit_func.SetParameter((j*3)+2,theta_sigma)  
                # fit_range = [theta_start-1,theta_start + (theta_step2*n_runs)+1]
                
    hist.Fit(total_fit_func,'','',fit_range[0],fit_range[1])


           


    print('CHI/NDF of GAUSSIAN FIT: ',(total_fit_func.GetChisquare()/total_fit_func.GetNDF()),', Prob ', total_fit_func.GetProb())
    for j in range(n_runs): 
        amplitude.append(total_fit_func.GetParameter((j*3))/hist.GetEntries())
        amplitude_error.append(total_fit_func.GetParError((j*3))/hist.GetEntries())
        mean.append(total_fit_func.GetParameter((j*3)+1))
        mean_error.append(total_fit_func.GetParError((j*3)+1))
        sigma.append(abs(total_fit_func.GetParameter((j*3)+2)))
        # sigma.append((total_fit_func.GetParameter((j*3)+2)))
        sigma_error.append(total_fit_func.GetParError((j*3)+2))
        func_name = 'funct_'+str(j)
        Gauss[j] = rt.TF1(func_name,'[0]*exp(-0.5*((x-[1])/[2])**2)',fit_range[0],fit_range[1])
        Gauss[j].SetParameters(total_fit_func.GetParameter(j*3),mean[j],sigma[j])
        if setting != 'TB':
            if j == 9:
                Gauss[j].SetLineColor(40)
            else:
                Gauss[j].SetLineColor(j+1)
            Gauss[j].SetNpx(1000)
            Gauss[j].Draw('same')
        Sigma_vs_Angle.SetPoint(j,mean[j],sigma[j])
        Sigma_vs_Angle.SetPointError(j,0,sigma_error[j])

    canvas1.SaveAs('Test.pdf','pdf')
    canvas1.SaveAs('Test.root','root')
    canvas2.cd()
    
    Sigma_vs_Angle_mean.SetPoint(0,min(mean),np.mean(sigma))
    Sigma_vs_Angle_mean.SetPoint(1,max(mean),np.mean(sigma))
    Sigma_vs_Angle.SetMarkerStyle(21)
    Sigma_vs_Angle.Draw('AP')
    Sigma_vs_Angle_mean.SetLineColor(2)
    if setting == "PHI":
        # Sigma_vs_Angle.SetTitle("Sigma_vs_Angle;phi (rad);sigma  (rad)")
        Sigma_vs_Angle.GetYaxis().SetRangeUser(0,1)
        Sigma_vs_Angle.SetTitle("Sigma_vs_Angle;phi (deg);sigma  (deg)")
        # Sigma_vs_Angle_mean.GetXaxis.SetTitle('phi (rad)')
        # Sigma_vs_Angle_mean.GetYaxis.SetTitle('sigma  (rad)')
    elif setting =="THETA":
        Sigma_vs_Angle.GetYaxis().SetRangeUser(0,0.15)
        Sigma_vs_Angle.SetTitle("Sigma_vs_Angle;theta (deg);sigma  (deg)")
        # Sigma_vs_Angle_mean.GetXaxis.SetTitle('Theta (deg)')
        # Sigma_vs_Angle_mean.GetYaxis.SetTitle('sigma  (deg)')


    Sigma_vs_Angle_mean.SetLineWidth(3)
    Sigma_vs_Angle_mean.Draw("Lsame")
    canvas2.SaveAs('Sigma_vs_angle.pdf','pdf')
    canvas2.SaveAs('Sigma_vs_angle.root','root')

    if configuration == "PARALLEL":
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
    elif configuration == "PERPENDICULAR":
        sigma_calbirated= []
        sigma_calbirated_error= []
        calib_parameter = 1
        print ('CALIBRATION PARAMETER = ', calib_parameter,' deg/u.a. ')


    for sigmas in sigma : 
        sigma_calbirated.append(sigmas*calib_parameter)
    for error in sigma_error :
        sigma_calbirated_error.append(error*calib_parameter)
    print ('amplitudes \t', amplitude)
    print ('means \t\t', mean)
    if configuration == "PARALLEL":
        print ('expected means \t', TB_peak_dist)
        print ('sigmas calibrated', sigma_calbirated , '\u00B1',sigma_calbirated_error )
        print ('AVERAGE sigma CALIBRATED: ', np.mean(sigma_calbirated),'\u00B1',np.mean(sigma_calbirated_error),' mm')
    elif configuration == "PERPENDICULAR":
        if setting == "PHI":
            # print ('expected means \t', phi_radiants )
            print ('expected means \t', phi_array )
            print ('sigmas calibrated', sigma_calbirated , '\u00B1',sigma_calbirated_error )
            # print ('AVERAGE sigma CALIBRATED: ', np.mean(sigma_calbirated),'\u00B1',np.mean(sigma_calbirated_error),' rad')
            print ('AVERAGE sigma CALIBRATED: ', np.mean(sigma_calbirated),'\u00B1',np.mean(sigma_calbirated_error),' deg')
            # print ('AVERAGE sigma CALIBRATED: ', np.mean(sigma_calbirated)*1000*np.pi/180,'\u00B1',np.mean(sigma_calbirated_error)*1000*np.pi/180,' mrad')
        elif setting == "THETA":
            print ('expected means \t', theta_array)
            print ('sigmas calibrated', sigma_calbirated , '\u00B1',sigma_calbirated_error )
            print ('AVERAGE sigma CALIBRATED: ', np.mean(sigma_calbirated),'\u00B1',np.mean(sigma_calbirated_error),' deg')
            print ('AVERAGE sigma CALIBRATED: ', np.mean(sigma_calbirated)*1000*np.pi/180,'\u00B1',np.mean(sigma_calbirated_error)*1000*np.pi/180,' mrad')

    return(amplitude,amplitude_error,mean,mean_error,sigma,sigma_error,sigma_calbirated,sigma_calbirated_error,calib_parameter)
if residuals == True : 
    f_s = open ("Residuals_scan.txt","a")
for i in range (0,len(names)):  
    position_calibrated = [] 
    path_name = path_names[i]# +'.root'
    print (path_name)
    f = rt.TFile.Open(path_name)
    canvas = rt.TCanvas(canvas_names[i],canvas_titles[i],2880,1800)
    canvas2 = rt.TCanvas('canvas2','Sigma_vs_Angle',2880,1800)
    hist_entries    = []
    hist_max        = []
    hist_mean       = []
    hist_std        = [] 
    for k in range(0,n_runs) :
        if setting == "G4":
            name = 'Hoff_run'+str(k)+'_3_11'   
            # hist = rt.TH1D()
            hist = f.Get(name)
            values = find_the_values(hist)
            hist_entries.append(values[0])
            hist_max.append(values[1])
            if k == 0:
                hist_mean.append(-1.39) ##first peak is covered by background, so we should not trust that value 
            else:
                hist_mean.append(values[2])
            hist_std.append(values[3])
        elif setting == "TB":
            hist_mean = [-1.39,-1.06,-0.73,-0.4,-0.06,0.23,0.53]
            hist_max = [334,1100,1550,1625,1458,1050,468]

    print ('entries: ',hist_entries, ',\nmax: ', hist_max,',\nmean: ', hist_mean,',\nstd: ', hist_std)
    result = peak_fitter(canvas,canvas2,f.Get(names[i]),rebin_value,n_runs,hist_mean,hist_max)

    if configuration == "PARALLEL":
        print('\nAVERAGE sigma :', np.mean(result[-3]), ' \u00B1 ', np.mean(result[-2]),' mm')  
    elif configuration == "PERPENDICULAR":
        if setting == "PHI":
            # print('\nAVERAGE sigma :', np.mean(result[-3]), ' \u00B1 ', np.mean(result[-2]),' rad') 
            # print('AVERAGE sigma :', np.mean(result[-3])*1000, ' \u00B1 ', np.mean(result[-2])*1000,' mrad\n') 
            print('\nAVERAGE sigma :', np.mean(result[-3]), ' \u00B1 ', np.mean(result[-2]),' deg') 
            print('AVERAGE sigma :', np.mean(result[-3])*1000*np.pi/180, ' \u00B1 ', np.mean(result[-2])*1000*np.pi/180,' mrad\n') 

        elif setting == "THETA":
            print('\nAVERAGE sigma :', np.mean(result[-3]), ' \u00B1 ', np.mean(result[-2]),' deg') 
            print('AVERAGE sigma :', np.mean(result[-3])*1000*np.pi/180, ' \u00B1 ', np.mean(result[-2])*1000*np.pi/180,' mrad\n') 


    calibration_parameter = result[-1]
    for mean in result[2]: position_calibrated.append(mean*calibration_parameter)
    print('Fitted position :',position_calibrated)
    diff = [] 
    for j in range(len(position_calibrated)-1):
        diff.append(position_calibrated[j+1]-position_calibrated[j])
    if configuration == "PARALLEL":
        print('peak dist ', diff ,' mm')
        print('AVERAGE peak distance ', np.mean(diff),' mm')
    elif configuration == "PERPENDICULAR":
        if setting == "PHI":
            print('peak dist ', diff ,' rad')
            # print('AVERAGE peak distance ', np.mean(diff),' rad')
            # print('AVERAGE peak distance ', np.mean(diff)*1000,' mrad')
            print('AVERAGE peak distance ', np.mean(diff),' deg')
            print('AVERAGE peak distance ', np.mean(diff)*1000*np.pi/180,' mrad')

        elif setting == "THETA":
            print('peak dist ', diff ,' deg')
            print('AVERAGE peak distance ', np.mean(diff),' deg')
            print('AVERAGE peak distance ', np.mean(diff)*1000*np.pi/180,' mrad')
    # input('press a key REMEMBER TO COPY AMPLITUDE LINE ON OFFSET PLOTTER! ')
    # input('continue')
    f.Close()
    if plot_offsets and setting == "G4":
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
        h1_bins = int(h1.GetNbinsX())
        h2_bins = int(h2.GetNbinsX())
        print('h1_entries: ', h1_entries,' -  h2_entries:', h2_entries)
        print ('h1_bins: ', h1_bins,'- h2 bins',h2_bins )
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
        print ('hSum bins: ', int(hSum.GetNbinsX()))
        if (int(h1.GetNbinsX()) == int(hSum.GetNbinsX())):    
            Chi_hist = 0
            bin_min = 25
            bin_max = 130
            for o in range(bin_min,bin_max):
                TB_bin_value = h1.GetBinContent(o)
                G4_bin_value = hSum.GetBinContent(o)
                if TB_bin_value != 0:
                    Chi_hist += ((G4_bin_value-TB_bin_value)*(G4_bin_value-TB_bin_value)/(TB_bin_value))
                else:
                    print (o,') TB_bin_value :', TB_bin_value)
        else:
            input ('DIFFERENT BIN NUM')        
        print ('Chi_hist ', Chi_hist)
        # string = 'Smearing: ' + str(Smearing) +', Par 0: ' + str(funct.GetParameter(0)) + ', Par1: ' + str(funct.GetParameter(1)) + ', EPar 0: ' + str(funct.GetParError(0)) +', EPar 1: ' + str(funct.GetParError(1)) + ', CHI: '+ str(funct.GetChisquare()) + ', NDF: ' + str(funct.GetNDF()) +'\n'
        string = 'Smearing: ' + str(Smearing) +', Par 0: ' + str(funct.GetParameter(0)) + ', Par1: ' + str(funct.GetParameter(1)) + ', EPar 0: ' + str(funct.GetParError(0)) +', EPar 1: ' + str(funct.GetParError(1)) + ', CHI: '+ str(Chi_hist) + ', NDF: ' + str(bin_max - bin_min -21) +'\n'
        print(string)

        # ratio_scale =[]
        # for rat in ratio:
        #     ratio_scale.append(rat*(h1_entries/h2_entries))
        # print ('ratios \"scaled\"', ratio_scale)
        if residuals == True : 
            f_s.write(string)
if residuals == True : 
    f_s.close()

# print ("POS couple ". result[2])
input('press a key') 
