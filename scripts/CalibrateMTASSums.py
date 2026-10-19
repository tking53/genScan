#!@Python3_EXECUTABLE@
import argparse
import yaml
import re
import numpy as np
import xml.etree.ElementTree as ET

def ParsePeaks(file):
    with open(file,'r') as f:
        data = yaml.safe_load(f)
    pks = {}
    for result in data['FitResults']:
        hisname = result['HisName']
        sumid = int(hisname.rsplit("_x", 1)[1])
        vals = result["Values"]
        numMean = 0 
        pk = 0.0
        for k,v in vals.items():
            m = re.fullmatch(r"Mean(\d{0,})",k)
            if m:
                pk += v
                numMean += 1
        pk /= numMean
        pks[sumid] = pk
    return pks

def ParsePeakOldCenter(file):
    with open(file,'r') as f:
        data = yaml.safe_load(f)
    pks = {}
    for result in data['FitResults']:
        hisname = result['HisName']
        vals = result["Values"]
        numMean = 0 
        pk = 0.0
        for k,v in vals.items():
            m = re.fullmatch(r"Mean(\d{0,})",k)
            if m:
                pk += v
                numMean += 1
        pk /= numMean
        for sumid in range(1,7):
            pks[sumid] = pk
    return pks

def GetCurrScaleFactors(file):
    with open(file) as f:
        firstline = f.readline()
    ncols = len(firstline.split())

    # dtype: first 3 int32, rest float64
    dtype = ['i4', 'i4', 'i4'] + ['f8'] * (ncols - 3)

    return np.genfromtxt(file, dtype=dtype)

def GetScale(pks,pk):
    scale = {}
    for k,v in pks.items():
        scale[k] = pk/v
    return scale

def GetPMTLocations(file,typename):
    tree = ET.parse(file)
    root = tree.getroot()
    mapnode = root.find('Map')
    pmts = {}
    for crate in mapnode.findall('Crate'):
        cratenum = int(str(crate.get('number')))
        for module in crate.findall('Module'):
            modnum = int(str(module.get('number')))
            for channel in module.findall('Channel'):
                channum = int(str(channel.get('number')))
                t = channel.get('type')
                if "mtas" in str(t):
                    st = channel.get('subtype')
                    if typename in str(st):
                        tt = channel.get('tags')
                        g = channel.get('group')
                        if "front" in str(tt):
                            pmts[2*(int(str(g))-1)+1] = [cratenum,modnum,channum]
                        elif "back" in str(tt):
                            pmts[2*(int(str(g))-1)+2] = [cratenum,modnum,channum]
                        else:
                            raise RuntimeError("MTAS Channel isn't tagged front or back")
    return pmts


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Script to rip apart all the MTAS Calibration files')
    parser.add_argument('-c','--c-file',type=str,default='CCal.yaml',help='yaml file used to calibrate centers')
    parser.add_argument('-i','--imo-file',type=str,default='IMOCal.yaml',help='yaml file used to calibrate inners')
    parser.add_argument('-p','--peak',type=float,default=1460.820,help='peak centroid used to determine the calibration factor')
    parser.add_argument('-t','--txtfile',default="MTASCal.txt",help='txt file that will be parsed to generate the output file, is output by CalibrateMTASPMTs')
    parser.add_argument('-x','--xmlfile',help='xml config file that will be parsed to generate the output file')
    parser.add_argument('-y','--output',default='MTASSumCal.txt',help='File used to store the calibration parameters for MTAS')
    parser.add_argument('-o','--old-center',type=bool,default=True,help='treat as old center or not')
    args = parser.parse_args()

    
    if args.old_center:
        cpeaks = ParsePeakOldCenter(args.c_file)
    else:
        cpeaks = ParsePeaks(args.c_file)
        
    cscale = GetScale(cpeaks,args.peak)

    imopeaks = ParsePeaks(args.imo_file)
    imoscale = GetScale(imopeaks,args.peak)

    currvals = GetCurrScaleFactors(args.txtfile)

    cpmts = GetPMTLocations(args.xmlfile,'center')
    ipmts = GetPMTLocations(args.xmlfile,'inner')
    mpmts = GetPMTLocations(args.xmlfile,'middle')
    opmts = GetPMTLocations(args.xmlfile,'outer')

    allscale = {}
    for k,v in cscale.items():
        allscale[2*(k-1)] = v 
        allscale[2*(k-1)+1] = v 
    for k,v in imoscale.items():
        allscale[2*(k-1)] = v 
        allscale[2*(k-1)+1] = v 

    for pmtid,cv in enumerate(currvals):
        currvals[pmtid][4] *= allscale[pmtid]

    allcorrect = True
    for idx,v in enumerate(cpmts.values()):
        tv = [int(currvals[idx][0]),int(currvals[idx][1]),int(currvals[idx][2])] 
        if v != tv:
            allcorrect = False
    for idx,v in enumerate(ipmts.values()):
        tv = [int(currvals[idx+12][0]),int(currvals[idx+12][1]),int(currvals[idx+12][2])] 
        if v != tv:
            allcorrect = False
    for idx,v in enumerate(mpmts.values()):
        tv = [int(currvals[idx+24][0]),int(currvals[idx+24][1]),int(currvals[idx+24][2])] 
        if v != tv:
            allcorrect = False
    for idx,v in enumerate(opmts.values()):
        tv = [int(currvals[idx+36][0]),int(currvals[idx+36][1]),int(currvals[idx+36][2])] 
        if v != tv:
            allcorrect = False
    if allcorrect:
        with open(args.output,'w') as f:
            for v in currvals:
                f.write(f"{v[0]} {v[1]} {v[2]} {v[3]} {v[4]}\n")
    else:
        print('txt layout is not the same as that from xml file')
