#!/bin/python3
import concurrent.futures
import subprocess
import sys
import threading
import argparse
import os

def run_genscan(semaphore,exe,input,output,config,filetype,root,port):
    with semaphore:
        full_command=[exe,"-c",f"{config}","-o",f"{output}","-x",f"{filetype}","-t",f"{root}","-p",f"{port}",f"{input}"]
        result = subprocess.run(full_command,capture_output=True)
        print(result.stdout)
        return result.stdout

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Parallel scan using genscan')
    parser.add_argument('files',metavar='FILE',type=str,nargs='+',help='file to use as input to GenScanor')
    parser.add_argument('-n','--nthreads',type=int,default=5,help='number of threads to use')
    parser.add_argument('-c','--config',type=str,help='config file used by genscan')
    parser.add_argument('-x','--filetype',type=str,help='filetype passed to genscan')
    parser.add_argument('-t','--tree',type=str,help='roottree passed to genscan')
    parser.add_argument('-o','--outputdir',type=str,help='outputdir, output files will have same name as the input')
    parser.add_argument('-e','--executable',default='GenScanor',type=str,help='genscanor executable to use')
    args = parser.parse_args()

    files = args.files
    nthreads = args.nthreads

    ports = [ 9090 + i%nthreads for i in range(len(files))]
    outputs = [ args.outputdir+os.path.splitext(os.path.basename(x))[0] for x in files ]
    config = [ args.config for i in files ]
    filetype = [ args.filetype for i in files ]
    root = [ args.tree for i in files ]
    max_concurrent_commands = args.nthreads
    semaphore = threading.Semaphore(max_concurrent_commands)
    commands = zip(files,outputs,config,filetype,root,ports)
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_concurrent_commands) as executor:
        futures = [executor.submit(run_genscan,semaphore,args.executable,input,output,config,filetype,root,port) for input, output, config, filetype, root, port in commands]
        for future in concurrent.futures.as_completed(futures):
            print(future.result)
