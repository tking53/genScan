#!@Python3_EXECUTABLE@
import concurrent.futures
import subprocess
import threading
import argparse
import os

def run_genscan(semaphore,exe,input,output,config,filetype,root,port,limit,crate,board,channel):
    with semaphore:
        full_command=[exe,"-c",f"{config}","-o",f"{output}","-x",f"{filetype}","-t",f"{root}","-p",f"{port}","-l",f"{limit}","-i",f"{crate}","-j",f"{board}","-k",f"{channel}",f"{input}"]
        result = subprocess.run(full_command,capture_output=True)
        print(result.stdout)
        return result.stdout

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Parallel scan using genscan')
    parser.add_argument('files',metavar='FILE',type=str,nargs='+',help='file to use as input to GenScanor')
    parser.add_argument('-n','--nthreads',type=int,default=5,help='number of threads to use')
    parser.add_argument('-i','--max_crates',type=int,default=1,help='number of crates to expect')
    parser.add_argument('-j','--max_slots',type=int,default=13,help='number of boards to expect')
    parser.add_argument('-k','--max_channels',type=int,default=16,help='number of channels to expect')
    parser.add_argument('-l','--limit',type=int,default=10,help='number of events to keep in history')
    parser.add_argument('-c','--config',type=str,help='config file used by genscan')
    parser.add_argument('-x','--filetype',type=str,help='filetype passed to genscan')
    parser.add_argument('-t','--tree',type=str,help='roottree passed to genscan')
    parser.add_argument('-o','--outputdir',type=str,help='outputdir, output files will have same name as the input')
    parser.add_argument('-e','--executable',default='GenScanor',type=str,help='genscanor executable to use')
    args = parser.parse_args()

    files = args.files
    nthreads = args.nthreads

    ports = [ -1 for _ in files]
    outputs = [ os.path.normpath(args.outputdir)+'/'+os.path.splitext(os.path.basename(x))[0] for x in files ]
    limits = [ args.limit for _ in files ]
    crates = [ args.max_crates for _ in files ]
    slots = [ args.max_slots for _ in files ]
    channels = [ args.max_channels for _ in files ] 
    config = [ args.config for _ in files ]
    filetype = [ args.filetype for _ in files ]
    root = [ args.tree for _ in files ]
    max_concurrent_commands = args.nthreads
    semaphore = threading.Semaphore(max_concurrent_commands)
    commands = zip(files,outputs,config,filetype,root,ports,limits,crates,slots,channels)
    print("WARNING, THIS SCRIPT WILL LOOK LIKE IT ISN'T WORKING. CTRL-C WILL NOT KILL IT, BUT CTRL-Z AND KILL %1 WILL. I PROMISE IT IS WORKING")
    print("MAKE SURE TO RUN ME IN A TMUX, AND OPEN UP TOP/HTOP TO WATCH THAT THINGS ARE GOING, tail -f *.log TO VERIFY THAT THINGS ARE RUNNING.")
    print("NO VISUALIZATION WILL BE AVAILABLE DURING THIS. SO spy.cxx WILL NOT BE OF ANY USE TO YOU")
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_concurrent_commands) as executor:
        futures = [executor.submit(run_genscan,semaphore,args.executable,input,output,config,filetype,root,port,limit,crate,board,channel) for input, output, config, filetype, root, port, limit, crate, board, channel in commands]
        for future in concurrent.futures.as_completed(futures):
            print(future.result)
