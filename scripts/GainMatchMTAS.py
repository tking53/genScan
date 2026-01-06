#!@Python3_EXECUTABLE@
import yaml
import numpy as np
import argparse
import subprocess
import os
import sys

def GetNamedValue(file: str,key: str):
    full_command = ["GenKeyPrint","-i",f"{file}","-k",f"{key}"]
    result = subprocess.run(full_command,capture_output=True)

    return int(result.stdout.decode())

def AddBound(cmd: list,bounds: list,name: str):
    cmd.append("-b")
    cmd.append(f"{name}:{bounds[0]}:{bounds[1]}")

def AddProjectionIndex(cmd: list,idx: int):
    cmd.append("-p")
    cmd.append(f"{idx}")

def LocatePeaks(file: str,data: str,indices: list,prefix: str,length: int,sigma: int,threshold: float):
    yfile = f"{prefix}.yaml"
    full_command = ["GenPeakLocator","-i",f"{file}","-d",f"{data}","-l",f"{length}","-s",f"{sigma}","-t",f"{threshold}","-o",f"{yfile}"]
    for idx in indices:
        AddProjectionIndex(full_command,idx)
    result = subprocess.run(full_command,capture_output=True)

    while True:
        with open(yfile,'r') as f:
            y = yaml.safe_load(f)

        bad_indices = []
        pks = []
        for idx,pk in enumerate(y['Peaks']):
            pk_loc = pk['Locations']
            if len(pk_loc) == 0 or len(pk_loc) > 1:
                bad_indices.append(idx)
            else:
                pks.append(pk_loc[0]['Energy'])
        if len(bad_indices) == 0:
            return pks
        else:
            if threshold < 0.5:
                threshold += 0.1;
                full_command[10] = f"{threshold}"
                continue
            elif sigma < 200:
                sigma += 10
                full_command[8] = f"{sigma}"
                continue
            elif length < 50:
                length += 5 
                full_command[6] = f"{length}"
                continue
            else:
                for idx in bad_indices:
                    print(f"unable to find any peaks for projection: {idx}")
                    return None
            
def DoSingleFit(file: str,data: str,idx: int,prefix: str,area: list,compton: list,pk: float,sigma: list,slope: list,offset: list,r: list,nfails: int):
    #for a good 1460, usually fit from 1200 to 1650
    #let's just try 10% left and right to begin with
    fit_range = []
    if pk < 500:
        fit_range = [0.95*pk,1.05*pk]
    elif pk >= 500 and pk < 1000:
        fit_range = [0.92*pk,1.08*pk]
    elif pk >= 1000 and pk < 2000:
        fit_range = [0.87*pk,1.13*pk]
    elif pk >= 2000 and pk < 3000:
        fit_range = [0.84*pk,1.16*pk]
    elif pk >= 3000 and pk < 4000:
        fit_range = [0.81*pk,1.19*pk]
    elif pk >= 5000  and pk < 6000:
        fit_range = [0.79*pk,1.22*pk]
    else:
        fit_range = [0.8*pk,1.2*pk]

    yfile = f"{prefix}_{idx}Report.yaml"

    currfails = 0
    while True:
        full_command = ["GenPeakFit","-m","2","-d",f"{data}","-l",f"{fit_range[0]}","-u",f"{fit_range[1]}","-o",f"{prefix}_{idx}","-i",f"{file}"]
        full_command.append("-p")
        full_command.append(f"{idx}")
        AddBound(full_command,area,"Area")
        AddBound(full_command,compton,"ComptonArea")
        AddBound(full_command,fit_range,"Mean")
        AddBound(full_command,sigma,"Sigma")
        AddBound(full_command,slope,"BkgSlope")
        AddBound(full_command,offset,"BkgOffset")
        result = subprocess.run(full_command,capture_output=True)
        if result.stdout:
            print(result.stdout.decode())

        with open(yfile,'r') as f:
            y = yaml.safe_load(f)

        red_chi2 = y['FitResults'][0]['ReducedChi2']
        lower_chi2 = 0.5
        upper_chi2 = 2
        if red_chi2 < lower_chi2 or red_chi2 > upper_chi2:
            currfails += 1
            print(f"projection: {idx} has issues fitting ReducedChi2: {red_chi2}, trying again")
            if currfails > nfails:
                print(f"projection: {idx} has exceeded acceptable number of fails")
                print(f"Bailing now on it, not changing the voltage")
                r.append(None)
                return
            fit_compton = y['FitResults'][0]['Values']['ComptonArea']
            if abs(fit_compton-compton[0]) < 1.0e-3:
                compton[0] *= 1.1
                print(f"raising ComptonArea lower bound to {compton[0]}")
                continue
            elif abs(fit_compton-compton[1]) < 1.0e-3:
                compton[1] *= 0.9
                print(f"lowering ComptonArea lower bound to {compton[1]}")
                continue
            else:
                fit_range = [0.97*fit_range[0],1.03*fit_range[1]]
                print(f"expanding fit range to [{fit_range[0]},{fit_range[1]}]")
                continue
        else:
            r.append(y['FitResults'][0]['Values']['Mean'])
            return

def CorrectMaskedPeaks(pks: list,indices: list,pk: float,failed_indices: list):
    for idx,cpk in enumerate(pks):
        if cpk is None:
            pks[idx] = pk
            failed_indices.append(indices[idx])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Script to generate fits and update the voltage file for MTAS',\
            formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('--area',type=float,nargs=2,default=[1.0,1.0e7],help='bounds for the Area')
    parser.add_argument('--compton-area',type=float,nargs=2,default=[1.0,1.0e7],help='bounds for the Area of the Compton')
    parser.add_argument('--sigma',type=float,nargs=2,default=[1.0,150.0],help='bounds for the sigma')
    parser.add_argument('--bkg-slope',type=float,nargs=2,default=[-10.0,0.0],help='bounds for the bkg slope')
    parser.add_argument('--bkg-offset',type=float,nargs=2,default=[1.0,1.0e3],help='bounds for the bkg offset')

    parser.add_argument('--chan-per-volt',type=int,default=10,help='number of channels per volt')
    parser.add_argument('--volt-delta-limit',type=int,default=50,help='maxmium voltage allowed to change, if exceed move by half')
    parser.add_argument('--num-fails',type=int,default=5,help='how many fails are allowed on a fit before it bails')
    #parser.add_argument('--imo-volt-limit',type=float,default=1450.0,help='upper limit for inner, middle, outer pmts')
    #parser.add_argument('--c-volt-limit',type=float,default=1250.0,help='upper limit for center pmts')

    parser.add_argument('-d','--data',type=str,default='Raw',help='2D histogram used to project the individual portions out of')
    parser.add_argument('-f','--facility',type=str,required=True,help='facility configuration FRIB/ANL')
    parser.add_argument('-l','--search-length',type=int,default=25,help='filter length used in determining the peak location')
    parser.add_argument('-o','--output',type=str,default='updated.ssv',help='name of the outputfile')
    parser.add_argument('-p','--peak',type=float,default=1460.820,help='peak centroid used to determine the calibration factor')
    parser.add_argument('-r','--root-file',type=str,required=True,help='name of the root file we need to fit and rip apart')
    parser.add_argument('-s','--search-sigma',type=int,default=200,help='sigma in bins of the peak used to determine peak location')
    parser.add_argument('-t','--search-threshold',type=int,default=0.5,help='threshold used in peak location')
    parser.add_argument('-v','--volt-file',type=str,required=True,help='name of the voltage file we need to update')

    args = parser.parse_args()
 
    #do this after parsing args so that help still works
    if "GENSCANSYS" not in os.environ:
        print("Error GENSCANSYS not in environment, bailing now!!!!")
        sys.exit(1)

    #these will be needed if we ever swap to pixie-32
    #or if we do messed up things with MTAS 
    #also causes a fault if genscan isn't in the path
    num_crates = GetNamedValue(args.root_file,'MAX_CRATES')
    num_cards_per_crate = GetNamedValue(args.root_file,'MAX_CARDS_PER_CRATE')
    num_channels_per_board = GetNamedValue(args.root_file,'MAX_CHANNELS_PER_BOARD')

    his_uid_map = { 0 :   'u0',  1:  'u1',   2:   'u2',  3:  'u3',   4:   'u4',   5:  'u5',
                        6 :   'u6',  7:  'u7',   8:   'u8',  9:  'u9',  10:  'u10',  11:  'u11',
                        12:  'u12', 13:  'u13', 14:  'u14', 15: 'u15',  16: 'u100',  17: 'u101',
                        18: 'u102', 19: 'u103', 20: 'u104', 21: 'u105', 22: 'u106',  23: 'u107',
                        24: 'u108', 25: 'u109', 26: 'u110', 27: 'u111', 28: 'u112',  29: 'u113',
                        30: 'u114', 31: 'u115', 32: 'u200', 33: 'u201', 34: 'u202',  35: 'u203',
                        36: 'u204', 37: 'u205', 38: 'u206', 39: 'u207', 40: 'u208',  41: 'u209',
                        42: 'u210', 43: 'u211', 44: 'u212', 45: 'u213', 46: 'u214',  47: 'u215'
                       }
        
    pmt_uid_map = { 
                       'C1F':   'u0', 'C1B':  'u1', 
                       'C2F':   'u2', 'C2B':  'u3',
                       'C3F':   'u4', 'C3B':  'u5',
                       'C4F':   'u6', 'C4B':  'u7',
                       'C5F':   'u8', 'C5B':  'u9',
                       'C6F':  'u10', 'C6B':  'u11',
                       'I1F':  'u12', 'I1B':  'u13', 
                       'I2F':  'u14', 'I2B':  'u15',
                       'I3F': 'u100', 'I3B': 'u101',
                       'I4F': 'u102', 'I4B': 'u103',
                       'I5F': 'u104', 'I5B': 'u105',
                       'I6F': 'u106', 'I6B': 'u107',
                       'M1F': 'u108', 'M1B': 'u109', 
                       'M2F': 'u110', 'M2B': 'u111',
                       'M3F': 'u112', 'M3B': 'u113',
                       'M4F': 'u114', 'M4B': 'u115',
                       'M5F': 'u200', 'M5B': 'u201',
                       'M6F': 'u202', 'M6B': 'u203',
                       'O1F': 'u204', 'O1B': 'u205', 
                       'O2F': 'u206', 'O2B': 'u207',
                       'O3F': 'u208', 'O3B': 'u209',
                       'O4F': 'u210', 'O4B': 'u211',
                       'O5F': 'u212', 'O5B': 'u213',
                       'O6F': 'u214', 'O6B': 'u215'
                       }

    reverse_pmt_uid_map = dict()
    for k,v in pmt_uid_map.items():
        reverse_pmt_uid_map[v] = k

    center_indices = []
    inner_indices = []
    middle_indices = []
    outer_indices = []

    if args.facility.upper() == 'FRIB': 
        center_indices = list(range(33,33+12)) 
        inner_indices = list(range(center_indices[-1]+1,center_indices[-1]+1+12))
        middle_indices = list(range(inner_indices[-1]+1,inner_indices[-1]+1+12))
        outer_indices = list(range(middle_indices[-1]+1,middle_indices[-1]+1+12))
    elif args.facility.upper() == 'ANL':
        center_indices = list(range(1,1+12)) 
        inner_indices = list(range(center_indices[-1]+1,center_indices[-1]+1+12))
        middle_indices = list(range(inner_indices[-1]+1,inner_indices[-1]+1+12))
        outer_indices = list(range(middle_indices[-1]+1,middle_indices[-1]+1+12))
    else:
        sys.exit(1)

    cpeaks = LocatePeaks(args.root_file,args.data,center_indices,'CenterPeaks',args.search_length,args.search_sigma,args.search_threshold)
    if cpeaks is None:
        sys.exit(1)
    ipeaks = LocatePeaks(args.root_file,args.data,inner_indices,'InnerPeaks',args.search_length,args.search_sigma,args.search_threshold)
    if ipeaks is None:
        sys.exit(1)
    mpeaks = LocatePeaks(args.root_file,args.data,middle_indices,'MiddlePeaks',args.search_length,args.search_sigma,args.search_threshold)
    if mpeaks is None:
        sys.exit(1)
    opeaks = LocatePeaks(args.root_file,args.data,outer_indices,'OuterPeaks',args.search_length,args.search_sigma,args.search_threshold)
    if opeaks is None:
        sys.exit(1)

    c = []
    for idx,pk in zip(center_indices,cpeaks):
        DoSingleFit(args.root_file,args.data,idx,'Center',args.area,args.compton_area,pk,args.sigma,args.bkg_slope,args.bkg_offset,c,args.num_fails)
    i = []
    for idx,pk in zip(inner_indices,ipeaks):
        DoSingleFit(args.root_file,args.data,idx,'Inner',args.area,args.compton_area,pk,args.sigma,args.bkg_slope,args.bkg_offset,i,args.num_fails)
    m = []
    for idx,pk in zip(middle_indices,mpeaks):
        DoSingleFit(args.root_file,args.data,idx,'Middle',args.area,args.compton_area,pk,args.sigma,args.bkg_slope,args.bkg_offset,m,args.num_fails)
    o = []
    for idx,pk in zip(outer_indices,opeaks):
        DoSingleFit(args.root_file,args.data,idx,'Outer',args.area,args.compton_area,pk,args.sigma,args.bkg_slope,args.bkg_offset,o,args.num_fails)

    failed_indices = []
    CorrectMaskedPeaks(c,center_indices,args.peak,failed_indices)
    CorrectMaskedPeaks(i,inner_indices,args.peak,failed_indices)
    CorrectMaskedPeaks(m,middle_indices,args.peak,failed_indices)
    CorrectMaskedPeaks(o,outer_indices,args.peak,failed_indices)

    cd = []
    for idx,pk in zip(center_indices,c):
        delta = (args.peak - pk)/args.chan_per_volt
        if abs(delta) > args.volt_delta_limit:
            delta /= 2.0
            print(f"Center PMT: {idx} exceeded delta limit [{args.volt_delta_limit}], reducing it to {delta}")
        else:
            print(f"Center PMT: {idx} within delta limit [{args.volt_delta_limit}], shifting it by {delta}")
        cd.append(delta)

    id = []
    for idx,pk in zip(inner_indices,i):
        delta = (args.peak - pk)/args.chan_per_volt
        if abs(delta) > args.volt_delta_limit:
            delta /= 2.0
            print(f"Inner PMT: {idx} exceeded delta limit [{args.volt_delta_limit}], reducing it to {delta}")
        else:
            print(f"Inner PMT: {idx} within delta limit [{args.volt_delta_limit}], shifting it by {delta}")
        id.append(delta)

    md = []
    for idx,pk in zip(middle_indices,m):
        delta = (args.peak - pk)/args.chan_per_volt
        if abs(delta) > args.volt_delta_limit:
            delta /= 2.0
            print(f"Middle PMT: {idx} exceeded delta limit [{args.volt_delta_limit}], reducing it to {delta}")
        else:
            print(f"Middle PMT: {idx} within delta limit [{args.volt_delta_limit}], shifting it by {delta}")
        md.append(delta)

    od = []
    for idx,pk in zip(outer_indices,o):
        delta = (args.peak - pk)/args.chan_per_volt
        if abs(delta) > args.volt_delta_limit:
            delta /= 2.0
            print(f"Outer PMT: {idx} exceeded delta limit [{args.volt_delta_limit}], reducing it to {delta}")
        else:
            print(f"Outer PMT: {idx} within delta limit [{args.volt_delta_limit}], shifting it by {delta}")
        od.append(delta)

    hadd_command = ["hadd","-f","CompleteFit.root"]
    yfiles = []
    for idx in center_indices:
        hadd_command.append(f"Center_{idx}.root")
        yfiles.append(f"Center_{idx}Report.yaml")
    for idx in inner_indices:
        hadd_command.append(f"Inner_{idx}.root")
        yfiles.append(f"Inner_{idx}Report.yaml")
    for idx in middle_indices:
        hadd_command.append(f"Middle_{idx}.root")
        yfiles.append(f"Middle_{idx}Report.yaml")
    for idx in outer_indices:
        hadd_command.append(f"Outer_{idx}.root")
        yfiles.append(f"Outer_{idx}Report.yaml")
    
    result = subprocess.run(hadd_command,capture_output=True)
    if result.stdout:
        print(result.stdout.decode())

    for f in hadd_command[3:]:
        cmd = ["rm","-f",f"{f}"]
        result = subprocess.run(cmd,capture_output=True)
        if result.stdout:
            print(result.stdout.decode())

    print('To view total result run the following command root \"$GENSCANSYS/scripts/DumpAllDrawable.cxx(\\\"CompleteFit.root\\\",\\\"^.*_proj_x[0-9]{1,2}$\\\",\\\"\\\",1.0,4000.0)\"')
    if len(failed_indices) > 0 :
        print(f'These are the following indices that failed to fit: {failed_indices}')
        print(f'it is likely that they are well within the fit and are just wet crystals which we have too')
        print(f'tight of a reduced chi2 bound on')
        print(f'Fit them with the following commands:')
        for idx in failed_indices:
            print(f'GenPeakFit -m 2 -p {idx} -l 1300.0 -u 1600.0 -i {args.root_file} -b "Mean:1400.0:1500.0" -b "Area:0.0:1.0e6" -b "Sigma:1.0:100.0" -b "ComptonArea:0.0:1.0e6" -b "BkgSlop:-10.0:0.0" -b "BkgOffset:0.0:1.0e6"')

    total_y = ""
    for idx,f in enumerate(yfiles):
        if idx == 0:
            cmd = ["cat",f"{f}"]
        else:
            cmd = ["tail","-n","+9",f"{f}"]
        result = subprocess.run(cmd,capture_output=True)
        total_y += result.stdout.decode()
        cmd = ["rm","-f",f"{f}"]
        result = subprocess.run(cmd,capture_output=True)
        if result.stdout:
            print(result.stdout.decode())

    with open('CompleteFitReport.yaml','w') as f:
        f.write(total_y)

    yesno = None
    while yesno not in ['Y','y','N','n']:
        yesno = input('Do these values look acceptable? (Y/y/N/n)')

    if yesno in ['Y','y']:
        data = np.genfromtxt(args.volt_file,names=['uid','modid','chanid','voltage','current','ramp'],dtype=None,encoding='utf-8')
        datadict = {}
        for item in data:
            datadict[item[0]] = {'modid':int(item[1]),'chanid':int(item[2]),'voltage':float(item[3]),'current':float(item[4]),'ramp':float(item[5])}

        Deltas = dict()
        Proj = dict()
        for idx,delta in zip(center_indices,cd):
            Deltas[idx-center_indices[0]] = delta
            Proj[idx-center_indices[0]] = f"{args.data}_proj_x{idx}"
        for idx,delta in zip(inner_indices,id):
            Deltas[idx-center_indices[0]] = delta
            Proj[idx-center_indices[0]] = f"{args.data}_proj_x{idx}"
        for idx,delta in zip(middle_indices,md):
            Deltas[idx-center_indices[0]] = delta
            Proj[idx-center_indices[0]] = f"{args.data}_proj_x{idx}"
        for idx,delta in zip(outer_indices,od):
            Deltas[idx-center_indices[0]] = delta
            Proj[idx-center_indices[0]] = f"{args.data}_proj_x{idx}"

        for k,v in Deltas.items():
            curruid = his_uid_map[k]
            currval = datadict[curruid]
            currpmt = reverse_pmt_uid_map[curruid]
            currvolt = currval['voltage']
            if( abs(v) > 0.5 ):
                newvolt = currvolt+v
                if( v < 0 ):
                    print(f'{Proj[k]} : {currpmt}: {currvolt}{v} -> {newvolt}')
                else:
                    print(f'{Proj[k]} : {currpmt}: {currvolt}+{v} -> {newvolt}')
                datadict[curruid]['voltage'] = newvolt
            else:
                print(f'{Proj[k]} : {currpmt} does not need to change')
        
        with open(args.output,'w') as ouf:
            ouf.write("## MPOD id , Module Number , Channel Number , Set Voltage , Set Current Limit , Set Ramp Rate (per module)\n")
            for k,d in datadict.items():
                ouf.write(f"{k} {d['modid']} {d['chanid']} {d['voltage']:.6f} {d['current']} {d['ramp']}\n")
    else:
        print('Not outputting anything, have a nice day :)')


