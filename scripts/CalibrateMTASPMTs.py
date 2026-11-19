#!@Python3_EXECUTABLE@
import argparse
import yaml
import re
import xml.etree.ElementTree as ET

def ParsePeaks(file):
    with open(file,'r') as f:
        data = yaml.safe_load(f)
    pks = {}
    for result in data['FitResults']:
        hisname = result['HisName']
        pmtid = int(hisname.rsplit("_x", 1)[1])
        vals = result["Values"]
        numMean = 0 
        pk = 0.0
        for k,v in vals.items():
            m = re.fullmatch(r"Mean(\d{0,})",k)
            if m:
                pk += v
                numMean += 1
        pk /= numMean
        pks[pmtid] = pk
    return pks

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

def AddScaleFactors(f,s,p):
    for k,v in s.items():
        f.write(f'{p[k][0]} {p[k][1]} {p[k][2]} 0.0 {v}\n')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Script to rip apart all the MTAS Calibration files')
    parser.add_argument('-c','--center-file',type=str,default='CenterCal.yaml',help='yaml file used to calibrate centers')
    parser.add_argument('-i','--inner-file',type=str,default='InnerCal.yaml',help='yaml file used to calibrate inners')
    parser.add_argument('-m','--middle-file',type=str,default='MiddleCal.yaml',help='yaml file used to calibrate middles')
    parser.add_argument('-o','--outer-file',type=str,default='OuterCal.yaml',help='yaml file used to calibrate outers')
    parser.add_argument('-p','--peak',type=float,default=1460.820,help='peak centroid used to determine the calibration factor')
    parser.add_argument('-x','--xmlfile',help='xml config file that will be parsed to generate the output file')
    parser.add_argument('-y','--output',default='MTASCal.txt',help='File used to store the calibration parameters for MTAS')
    args = parser.parse_args()

    cpeaks = ParsePeaks(args.center_file)
    cscale = GetScale(cpeaks,args.peak)

    ipeaks = ParsePeaks(args.inner_file)
    iscale = GetScale(ipeaks,args.peak)

    mpeaks = ParsePeaks(args.middle_file)
    mscale = GetScale(mpeaks,args.peak)

    opeaks = ParsePeaks(args.outer_file)
    oscale = GetScale(opeaks,args.peak)

    cpmts = GetPMTLocations(args.xmlfile,'center')
    ipmts = GetPMTLocations(args.xmlfile,'inner')
    mpmts = GetPMTLocations(args.xmlfile,'middle')
    opmts = GetPMTLocations(args.xmlfile,'outer')

    with open(args.output,'w') as f:
        AddScaleFactors(f,cscale,cpmts)
        AddScaleFactors(f,iscale,ipmts)
        AddScaleFactors(f,mscale,mpmts)
        AddScaleFactors(f,oscale,opmts)
