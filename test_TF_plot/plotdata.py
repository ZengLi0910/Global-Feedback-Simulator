def plotdata(plotcfgfile):

    import sys
    sys.path.append("../")
    import json
    from readjson.readjson import jsontodict
    import numpy as np
    import pylab as py
    plotdict=jsontodict(plotcfgfile) #read json and return python dictionary
    acceldict=jsontodict(plotdict["Accelerator Config"]) #read accelerator config

    data=loaddata(plotdict["Data"]) #read the data file and return array
   
    connect = acceldict['Accelerator']['connect']
    Nlinac=len(connect)

    closeflag=plotdict.get('closewindow','none')
    py.close(closeflag)

    for (typeplot,plotcont) in plotdict.iteritems():
        if "type" not in plotcont:
            continue

        if (plotcont["type"]=='TF'):

            outdata=keytocolumn(data,plotcont["output"],connect=connect)
            indata=keytocolumn(data,plotcont["input"],connect=connect)

            scale_out=plotcont["output"].get("scale",1.0)
            scale_in=plotcont["input"].get("scale",1.0)
            OL_suppression=plotcont.get("OL_suppression",1.0)

            dt=acceldict["Simulation"]["dt"]
            TF_plot(indata,outdata,dt,
                    scale_in=scale_in,scale_out=scale_in, 
                    OL_suppression=OL_suppression)
        elif (plotcont["type"]=='versus'):
            x=keytocolumn(data,plotcont["x"],connect=connect)
            y=keytocolumn(data,plotcont["y"],connect=connect)
             
            scale_x=plotcont["x"].get("scale",1.0)
            scale_y=plotcont["y"].get("scale",1.0)

            xtoplot=x*scale_x
            ytoplot=y*scale_y
            
            xlabelis=plotcont.get('xlabel',"{0} times {1}".format(plotcont['x']['quantity'],scale_x))
            ylabelis=plotcont.get('ylabel',"{0} times {1}".format(plotcont['y']['quantity'],scale_y))

            print 'versus'
        else:
            print 'Case for that type {0}  is not present'.format(plotcont["type"])

        try:
            fignum=plotcont.get('figure',1)
            linetype=plotcont.get('linetype','')
            linewidth=plotcont.get('linewidth',1)
            linelabel=plotcont.get('linelabel',None)
        
            py.figure(fignum)
            py.plot(xtoplot,ytoplot,linetype,label=linelabel,linewidth=linewidth)
            py.xlabel(xlabelis)
            py.ylabel(ylabelis)
            py.legend()
            py.show()
        except:
            print "Plot did not work likely something not right in your commands to parse data for plot command"


    return 0

def keytocolumn(data,iodict, connect=None):
    print connect
    if (connect):
        linidx = { connect[i]:i for i in range(len(connect)) }

    linackeys={
        "error_vol_a":0,
        "error_vol_p":1,
        "dE_E":2,
        "dtz":3,
        "cav_voltage":4,
        "fpga_err":5,
        "fpga_set_point":6
        }
    otherkeys={
        "t":0,
        "Q":1,
        "dQ_Q":2,
        "dtg":3,
        "dE_Elast":4,
        "dtzlast":5,
        "adc_noised":6
        }
    item=iodict["quantity"]
    if item in otherkeys:
        col=otherkeys[item]
    elif item in linackeys:
        if "linac" not in iodict:
            print "need a linac key word for this item"
            return False
        col=len(otherkeys)+linidx[iodict["linac"]]*len(linackeys) \
            +linackeys[item]
    else:
        print "that key does not have a translation"
        return 0
    print col
    return data[:,col]


def loaddata(datafile):
    import numpy as np
    
    arrayout=np.loadtxt(datafile,dtype=np.complex)

    return arrayout




def TF_plot(indata,outdata,dt,
            scale_in=1.0,scale_out=1.0, OL_suppression=1.0,
            figurenum=1, outputfile=None):

    from scipy import fft
    import numpy as np
    import pylab as py

    fft_in=fft(indata)
    fft_out=fft(outdata)

    TF_mag=(np.abs(fft_out)*scale_out)/(np.abs(fft_in)*scale_in)
    TF_mag=TF_mag/OL_suppression
    TF_mag=20*np.log10(TF_mag)

    TF_pha=(np.angle(fft_out)*scale_out)/(np.angle(fft_in)*scale_in)

    N=fft_in.size
    freq=np.arange(N)/(N*dt)
    

    N_nyq=np.int(N/2) # only frequencys up to half the sampling frequency are
                      #viable so divide in half and round down to cut out
                      #aliased results above 1/(2*dt)

    py.figure(1)
    py.plot(freq[0:N_nyq],20*np.log10(fft_out[0:N_nyq]))
    
    py.figure(2)
    py.plot(freq[0:N_nyq],TF_mag[0:N_nyq])
    
    py.figure(3)
    py.plot(freq[0:N_nyq],TF_pha[0:N_nyq]*180/np.pi)

    py.figure(4)
    py.plot(indata)
    print indata
    py.figure(5)
    py.plot(outdata)
    print outdata
    return 0

def movavg(data,Navg):
    import numpy as np
    N=data.size

    out=np.zeros(data.shape)
    for k in range(N):
        out[k]=np.mean(data[max(0,k-Navg/2):min(k+Navg/2,N-1)])

    return out
        

def test():
    import numpy as np
    import pylab as py
    from scipy import fft, ifft
    py.close('all')
    t=np.linspace(0,2*np.pi,num=20000,endpoint=True)
    
    y=np.ones(t.shape,dtype=float)
    y[0:5000]=0.0
    print y.shape
    #y=np.cos(1200*t*2*np.pi)#+np.cos(40*t*2*np.pi)+np.cos(100*t*2*np.pi)
    #y+=(np.random.rand(y.size)-.5)
    #y=np.floor(((t*100)%100)/10)

    x=movavg(y,100)

    TF_plot(y,x,t[1])
    print t[1]
    print 1/t[1]

    py.figure(4)
    py.plot(t,y)
    py.plot(t,x)

    #for k in range(100):
    #    y=np.cos(t*2*np.pi)
    #    fout=fft(y)
    #    fout[300:1700]=0.0
    #    x =ifft(yout)
        
    #    TF_
    return 0
