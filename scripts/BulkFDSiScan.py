#!@Python3_EXECUTABLE@
import concurrent.futures
import subprocess
import threading
import argparse
import os
import re

def run_genscan(semaphore,exe,input,output,config,root,port,limit,crate,board,channel):
    with semaphore:
        full_command=[exe,"-c",f"{config}","-o",f"{output}","-x","evt","-t",f"{root}","-p",f"{port}","-l",f"{limit}","-i",f"{crate}","-j",f"{board}","-k",f"{channel}"]
        for file in input:
            full_command.append(f"{file}")
        result = subprocess.run(full_command,capture_output=True)
        print(result.stdout)
        return result.stdout

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Parallel scan using genscan')
    parser.add_argument('inputdirs',metavar='INPUTDIRS',type=str,nargs='+',help='file to use as input to GenScanor')
    parser.add_argument('-n','--nthreads',type=int,default=5,help='number of threads to use')
    parser.add_argument('-i','--max_crates',type=int,default=1,help='number of crates to expect')
    parser.add_argument('-j','--max_slots',type=int,default=13,help='number of boards to expect')
    parser.add_argument('-k','--max_channels',type=int,default=16,help='number of channels to expect')
    parser.add_argument('-l','--limit',type=int,default=10,help='number of events to keep in history')
    parser.add_argument('-c','--config',type=str,help='config file used by genscan')
    parser.add_argument('-t','--tree',type=str,help='roottree passed to genscan')
    parser.add_argument('-o','--outputdir',type=str,help='outputdir, outputdirectory, all files will be')
    parser.add_argument('-e','--executable',default='GenScanor',type=str,help='genscanor executable to use')
    args = parser.parse_args()

    inputdirs = args.inputdirs
    nthreads = args.nthreads

    ports = [ -1 for _ in inputdirs]
    limits = [ args.limit for _ in inputdirs ]
    crates = [ args.max_crates for _ in inputdirs ]
    slots = [ args.max_slots for _ in inputdirs ]
    channels = [ args.max_channels for _ in inputdirs ] 
    config = [ args.config for _ in inputdirs ]
    roottree = [ args.tree for _ in inputdirs ]

    filelists = []
    outputs = []
    for d in inputdirs:
        currfilelist = []
        for root, dirs, files in os.walk(d):
            for file in files:
                if file.endswith('.evt'):
                    currfilelist.append(root+'/'+file)
            currfilelist = sorted(currfilelist,key=lambda x: int(x.split('-')[2].split('.')[0]))
            filelists.append(currfilelist)
        outputs.append(os.path.normpath(args.outputdir)+'/bulkscan_'+os.path.basename(os.path.normpath(d)))

    max_concurrent_commands = args.nthreads
    semaphore = threading.Semaphore(max_concurrent_commands)
    commands = zip(filelists,outputs,config,roottree,ports,limits,crates,slots,channels)
    print("WARNING, THIS SCRIPT WILL LOOK LIKE IT ISN'T WORKING. CTRL-C WILL NOT KILL IT, BUT CTRL-Z AND KILL %1 WILL. I PROMISE IT IS WORKING")
    print("MAKE SURE TO RUN ME IN A TMUX, AND OPEN UP TOP/HTOP TO WATCH THAT THINGS ARE GOING, tail -f *.log TO VERIFY THAT THINGS ARE RUNNING.")
    print("NO VISUALIZATION WILL BE AVAILABLE DURING THIS. SO spy.cxx WILL NOT BE OF ANY USE TO YOU")
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_concurrent_commands) as executor:
        futures = [executor.submit(run_genscan,semaphore,args.executable,input,output,config,root,port,limit,crate,board,channel) for input, output, config, root, port, limit, crate, board, channel in commands]
        for future in concurrent.futures.as_completed(futures):
            print(future.result)
