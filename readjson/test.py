import sys
sys.path.append("../") 

from readjson import *
#reload(readjson)
#from readjson import *
a,accelout,linp_arr,gun=loadaccelerator("footest.cfg")

print linp_arr

lam=readentry(a,a['c'])
blah= readentry(a,a['d_TRF2']['poles'][0][0])
