import numpy as np
# import matplotlib as mpl
# mpl.use('tkagg')
# import matplotlib.pyplot as plt
from array import array

import ROOT as rt


std_smearing  = []

def make_graph(n,x,y, Title,X_axis,Y_axis):
    gr = rt.TGraph(n)
    gr.SetTitle( Title )
    gr.GetXaxis().SetTitle( X_axis)
    gr.GetYaxis().SetTitle( Y_axis )
    for i in range( n ):
        gr.SetPoint(i,x[i],y[i])
    return gr   
def make_graph_error(n,x,y,ex,ey, Title,X_axis,Y_axis):
    gr = rt.TGraphErrors(n)
    gr.SetTitle( Title )
    gr.GetXaxis().SetTitle( X_axis)
    gr.GetYaxis().SetTitle( Y_axis )
    for i in range( n ):
        gr.SetPoint(i,x[i],y[i])
        gr.SetPointError(i,ex[i],ey[i])
    return gr   

def Target_depending_graph(target_string,target_array,x,ex,y,ey,Title,X_axis,Y_axis):
    # X_v, Y_v = array ( 'd' ), array ( 'd' )
    # print (target_string)
    # print(target_id[-1])
    gr_cond = rt.TGraphErrors(target_array.count(target_string))
    # print ('count ', target_array.count(target_string))
    print ('\n target string ', target_string,' target array', target_array,' x ',x,' ex ', ex ,' y ', y,' ey ',ey)
    j=0
    for i in range (len(target_array)):
        if target_array[i] == target_string:    
            print ("Condition: ", target_array[i],' i', i,', X_i ', x[i],', ex_i ', ex[i],' Y_i ', y[i],', ey_i ', ey[i])
            # X_v.append(x[i])
            # Y_v.append(y[i])
            gr_cond.SetPoint(j,x[i],y[i])
            gr_cond.SetPointError(j,ex[i],ey[i])
            j+=1    
    # n = target_array.count(target_string)
    # print ('n:',n,' X ', X_v ,', Y ', Y_v,'\n')        
    # gr_cond = rt.TGraph(n,X_v,Y_v)
    gr_cond.SetTitle( Title )
    gr_cond.GetXaxis().SetTitle( X_axis)
    gr_cond.GetYaxis().SetTitle( Y_axis )
    gr_cond.SetMarkerStyle( 4 )
    return gr_cond
# print('std_theta', std_theta)

with open("Residuals_scan.txt") as f:
    lines = f.readlines()
    # print ('Lines ', lines)

    Smearings = [line.split(': ')[1] for line in lines]
    # print ('Smearings ', Smearings)
    smearing_id    = [float(smearing.split(',')[0]) for smearing in Smearings]
    Esmearing_id    = [0.0 for smearing in Smearings]
    # print ('smearing_id ', smearing_id)
    # print ('Esmearing_id ', Esmearing_id)
    
    
    Par0 = [line.split(': ')[2] for line in lines]
    # print ('Par0 ', Par0)
    Par0_id   = [float(par0.split(',')[0]) for par0 in Par0]
    # print ('Par0_id ', Par0_id)

    Par1 = [line.split(': ')[3] for line in lines]
    # print ('Par1 ', Par1)
    Par1_id   = [float(par1.split(',')[0]) for par1 in Par1]
    # print ('Par1_id ', Par1_id)

    EPar0 = [line.split(': ')[4] for line in lines]
    # print ('EPar0 ', EPar0)
    EPar0_id   = [float(epar0.split(',')[0]) for epar0 in EPar0]
    # print ('EPar0_id ', EPar0_id)

    EPar1 = [line.split(': ')[5] for line in lines]
    # print ('EPar1 ', EPar1)
    EPar1_id   = [float(epar1.split(',')[0]) for epar1 in EPar1]
    # print ('EPar1_id ', EPar1_id)

    CHI = [line.split(': ')[6] for line in lines]
    # print ('CHI ', CHI)
    CHI_id   = [float(chi.split(',')[0]) for chi in CHI]
    # print ('CHI_id ', CHI_id)
    
    NDF = [line.split(': ')[7] for line in lines]
    # print ('NDF ', NDF)
    NDF_id   = [float(ndf.split(',')[0]) for ndf in NDF]
    # print ('NDF_id ', NDF_id)

Chi_reduced = []   
for chi in CHI_id:    Chi_reduced.append(chi/NDF_id[0])
# print ('red chi ', Chi_reduced)

gr0 = make_graph_error(len(smearing_id),smearing_id,Par0_id,Esmearing_id,EPar0_id,"dE_scan","dE","Residual Par0")
gr0Zoom = make_graph_error(len(smearing_id),smearing_id,Par0_id,Esmearing_id,EPar0_id,"dE_scan","dE","Residual Par0")
ref0_y=[1.0,1.0]
ref0_x=[min(smearing_id),max(smearing_id)]
# print ("rX ", ref0_x, ", rY ", ref0_y)
ref0 = make_graph(2,ref0_x,ref0_y,"dE_scan","dE","Residual Par0")

gr1 = make_graph_error(len(smearing_id),smearing_id,Par1_id,Esmearing_id,EPar1_id,"dE_Scan","dE","Residual Par1")
gr1Zoom = make_graph_error(len(smearing_id),smearing_id,Par1_id,Esmearing_id,EPar1_id,"dE_Scan","dE","Residual Par1")
ref1_y=[0.0,0.0]
ref1_x=[min(smearing_id),max(smearing_id)]
# print ("rX ", ref1_x, ", rY ", ref1_y)
ref1 = make_graph(2,ref1_x,ref1_y,"dE_scan","dE","Residual Par1")

zero = []
for zeros in smearing_id : zero.append(0)
grchi = make_graph_error(len(smearing_id),smearing_id,Chi_reduced,Esmearing_id,zero,"dE_Scan","dE","Residual - \chi^{2}/NDF")
grchiZoom = make_graph_error(len(smearing_id),smearing_id,Chi_reduced,Esmearing_id,zero,"dE_Scan","dE","Residual - \chi^{2}/NDF")
refchi_y=[1.0,1.0]
refchi_x=[min(smearing_id),max(smearing_id)]
# print ("rX ", refchi_x, ", rY ", refchi_y)
refchi = make_graph(2,refchi_x,refchi_y,"dE_scan","dE","Residual \Chi")




can=rt.TCanvas('can','dE_Resiudal_scan',2880,1800)
can.Divide(3,2)
can.cd(1)
gr0.Draw()
ref0.SetLineColor(2)
ref0.Draw('same')

can.cd(2)
gr1.Draw()
gr1.GetYaxis().SetRangeUser(-0.1,0.1)
ref1.SetLineColor(2)
ref1.Draw('same')

can.cd(3)
grchi.Draw()
# grchi.GetYaxis().SetRangeUser(-0.1,0.1)
refchi.SetLineColor(2)
refchi.Draw('same')

zoom_range_X =[0.21,0.23] 
can.cd(4)
gr0Zoom.Draw()
gr0Zoom.GetXaxis().SetRangeUser(zoom_range_X[0],zoom_range_X[1])
gr0Zoom.GetYaxis().SetRangeUser(0.9,1.12)
ref0.SetLineColor(2)
ref0.Draw('same')

can.cd(5)
gr1Zoom.Draw()
gr1Zoom.GetXaxis().SetRangeUser(zoom_range_X[0],zoom_range_X[1])
gr1Zoom.GetYaxis().SetRangeUser(-0.02,0.02)
ref1.SetLineColor(2)
ref1.Draw('same')

can.cd(6)
grchiZoom.Draw()
grchiZoom.GetXaxis().SetRangeUser(zoom_range_X[0],zoom_range_X[1])
grchiZoom.GetYaxis().SetRangeUser(-0.02,25)
refchi.SetLineColor(2)
refchi.Draw('same')

can.SaveAs('dE_scan.root','root')
can.SaveAs('dE_scan.pdf','pdf')

input ('press a key to exit')