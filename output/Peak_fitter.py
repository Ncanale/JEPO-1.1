import ROOT as rt
import numpy as np
import math as m
import time


configuration = "PERPENDICULAR"

if configuration == "PARALLEL":
    names = ['Hoff_3_11']
    path_names = ['Offsets.root']
    
    ## REAL
    # names = ['h_eta_offset']
    # path_names = ['TB_offset.root']
    
    canvas_names = ['can']
    canvas_titles =['Offset']
    spect_sigma = 1.5
    spect_th    = 0.5
    
    
elif configuration == "PERPENDICULAR":
    # names = ['HRa']
    # canvas_names = ['can_r']
    # canvas_titles =['R_distribution']
    # path_names = ['HRa.root']
    names = ['HPh']
    canvas_names = ['can_Ph']
    canvas_titles =['Ph_distribution']
    path_names = ['HPh.root']
    spect_sigma = 10
    spect_th    = 0.5
else:
    input ("CHECK THE CONFIGURATIONS")

rebin_value = 1 
n_runs      = 15


Target= "Empty"
Smearing=0.19
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

def peak_fitter(canvas,hist,rebin_value,n_runs,spect_sigma,spect_th):
    canvas.cd()
    hist.Draw()
    hist.Rebin(rebin_value)
    spect = rt.TSpectrum((n_runs))
    nfound = spect.Search(hist,spect_sigma,"nobackground",spect_th) #
    print('Found',nfound, 'candidate peaks to fit, sigma tSpect ',spect_sigma, ',th tSpect ',spect_th,'.\n')
    xpeaks = spect.GetPositionX()
    ypeaks = spect.GetPositionY()
    # xpeaks = [-15.6, -11.6, -8.35, 8.05, 11.93, 15.36]
    xpeak_py = []
    ypeak_py = []
    for j in range (nfound): #it creates a List from a root buffer
        # print ('Xpeak #',j,' ', xpeaks[j])
        xpeak_py.append(xpeaks[j])
        ypeak_py.append(ypeaks[j])
        # hist.Fit('gaus','','',xpeaks[j],xpeaks[j])
    # print('xpeak:' , xpeak_py)
    # pos_couple= [[xpeak_py[p],yp eak_py[p]] for p in range(len(xpeaks))]
    # ypeak_py.sort()
    # sorted(pos_couple, key=lambda k:[k[0],k[1]])
    print('xpeak :', xpeak_py)
    print ('ypeak ', ypeak_py)
    xpeak_py.sort()

    diff_list = [] 
    print(xpeak_py)
    if nfound >1:
        for x1, x2 in zip(xpeak_py[0::], xpeak_py[1::]): 
            print ('peaks',x1,', ',x2, ', Dx ',x2-x1)
            diff_list.append(x2-x1)
        diff_list.append(diff_list[-1])
        # # # Printing difference list # #
        print ("difference list: ", diff_list)
    else:
        for j in range (nfound):
            diff_list.append(0)

    peak_fit        = []
    peak_fit_error  = []
    sigma_fit       = []
    sigma_fit_error = []
    # fit_funcs       = []
    fit_parameters  = []
    # # # print(fit_funcs)
    for j in range(nfound):   
        funct = rt.TF1('funct','gaus') 
        if nfound > 1:
            hist.Fit(funct,'IQ','',xpeak_py[j]-(np.mean(diff_list)/2),xpeak_py[j]+(np.mean(diff_list[j])/2))
        else:
            hist.Fit(funct,'IQ','',xpeak_py[j] - 0.5, xpeak_py[j] + 0.5)
        # print('##########',j,'range of fit: (',xpeak_py[j]-(diff_list[j]/2),',',xpeak_py[j]+(diff_list[j]/2),') ##########')
        # print (funct.GetParameter(1),funct.GetParameter(2))
        peak_fit.append(funct.GetParameter(1))
        peak_fit_error.append(funct.GetParError(1))
        sigma_fit.append(funct.GetParameter(2))
        sigma_fit_error.append(funct.GetParError(2))
  

        fit_parameters.append(funct.GetParameter(0))
        fit_parameters.append(funct.GetParameter(1))
        fit_parameters.append(funct.GetParameter(2))
    # can_peak=rt.TCanvas('can_peak','peaks',1800,1800)
    # can_peak.cd()
    funct_array =[]
    for j in range(nfound):  
        name = 'draw_peak_gauss['+str(j)+']'
        # print('name "', name,'"')
        # draw_peak_gauss = rt.TF1(name,'[0]*exp(-0.5*((x-[1])*(x-[1]))/([2]*[2]))', xpeak_py[j]-(diff_list[j]/2),xpeak_py[j]+(diff_list[j]/2))
        draw_peak_gauss = rt.TF1(name,'[0]*exp(-0.5*((x-[1])*(x-[1]))/([2]*[2]))', xpeak_py[j]-(np.mean(diff_list)/2),xpeak_py[j]+(np.mean(diff_list)/2))
        # print (j,' fit par %.3f, %.3f, %.3f' % (fit_parameters[(3*j)],fit_parameters[(3*j)+1], fit_parameters[(3*j)+2]))
        draw_peak_gauss.SetParameter(0,fit_parameters[(3*j)])
        draw_peak_gauss.SetParameter(1,fit_parameters[(3*j)+1])
        draw_peak_gauss.SetParameter(2,fit_parameters[(3*j)+2])
        funct_array.append(draw_peak_gauss)
        # print('draw peak gauss param  %.3f, %.3f, %.3f,' % (draw_peak_gauss.GetParameter(0),draw_peak_gauss.GetParameter(1),draw_peak_gauss.GetParameter(2)))
        funct_array[j].Draw('same')
        canvas.Update()

    # # Calibration # #
    sigma= []
    calibration_edges =[ 5.181 ,24.538]
    if configuration == "PARALLEL":
        if nfound >1 :
            Dx  = (peak_fit[-2] - peak_fit[1])
            print('peak -2 ', peak_fit[-2] ,' peak 1 ', peak_fit[1],'; diff',Dx )
            distance = (calibration_edges[1]-calibration_edges[0])/10
            calib_parameter = distance/Dx
        else :
            calib_parameter = 1
            print (' UNCALIBRATED ')
    if configuration == "PERPENDICULAR":
        calib_parameter = 1

         
    # print('calib param',calib_parameter)
    # for k in range(1,nfound-1): #calibrating the resolution 
    for k in range(nfound): #calibrating the resolution 
        # print('sigma %.3f mm' % (sigma_fit[k]*calib_parameter))
        # sigma.append(sigma_fit[k])
        sigma.append(sigma_fit[k]*calib_parameter)
        sigmas_errors.append(sigma_fit_error[k]*calib_parameter)
    sigma_err = root_sum_square(sigmas_errors,1) #No calibration needed -> Calib param = 1 
    print ('sigmas : ', sigma,' \u00B1 ', sigmas_errors,' \n')
    
    print ('\naverage sigma value %.3f cm'% (np.mean(sigma)),', st dev sigma %.3f cm'% (np.std(sigma)) ,'root_sum_square %.3f cm'%(sigma_err),'\n')
    print ('sigma : ', sigma,' \u00B1 ', sigmas_errors,' \n')
    
    print (' positions ', peak_fit)
    
    canvas.SaveAs(names[0]+'.pdf','pdf')
    return [np.mean(sigma),sigma_err]#,pos_couple] 

f_s = open ("sigmas_run.txt","a")
for i in range (0,len(names)):  
    path_name = path_names[i]# +'.root'
    print (path_name)
    f = rt.TFile.Open(path_name)
    canvas = rt.TCanvas(canvas_names[i],canvas_titles[i],2880,1800)
    result = peak_fitter(canvas,f.Get(names[i]),rebin_value,n_runs,spect_sigma,spect_th)  
    sigmas.append(result)
   
   
    # print('\n (sigmas \u00B1 error) ', sigmas,' cm' )
    # print('\n (sigmas \u00B1 error) ', np.array(sigmas)*10, 'mm \n' )
    # string = canvas_titles[i]
    # string =string + (" - Energy: %s,Target: %s,Smearing: %.3f,Sigma: %.3f mm,Sigma_err: %.3f mm \n" % (Energy,Target,Smearing,result[0]*10,result[1]*10))
   
    print('\n (sigmas \u00B1 error) ', sigmas,' rad' )
    print('\n (sigmas \u00B1 error) ', np.array(sigmas)*1000,' mrad' )
    string = canvas_titles[i]
    string = string + (" - Energy: %s,Target: %s,Smearing: %.3f,Sigma: %.3f mrad,Sigma_err: %.3f mrad \n" % (Energy,Target,Smearing,result[0]*1000,result[1]*1000))
    print (string)
    f_s.write(string)
f_s.close()

# print ("POS couple ". result[2])
input('press a key')

