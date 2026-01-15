import argparse 
import re 
import os
import sys
from collections import defaultdict

def AddPathtoExclude(paths,path):
    if path[-1] == '/':
        path = path[:-2] 

    if paths is None:
        paths = [path]
    else:
        if path not in args.exclude and path+'/' not in args.exclude:
            args.exclude.append(path+'/')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Script to check for valid instantiation of histogram names throughtout codebase')
    parser.add_argument('--dir',type=str,required=True,help='top level directory to drill down through and extract histogram declarations')
    parser.add_argument('--exclude',nargs='+',help='subdirectory to add to ignore list')
    parser.add_argument('--extensions',nargs='+',default=['cpp','hpp','cxx','hxx'],help='file extensions to parse and check')
    args = parser.parse_args()

    AddPathtoExclude(args.exclude,'build')
    AddPathtoExclude(args.exclude,'.git')
    AddPathtoExclude(args.exclude,'.cache')
    AddPathtoExclude(args.exclude,'install')

    for idx,subdir in enumerate(args.exclude):
        if subdir[-1] != '/':
            args.exclude[idx] += '/'

    for idx,ext in enumerate(args.extensions):
        if ext[0] != '.':
            args.extensions[idx] = '.'+ext

    to_check = []
    for root, dirs, files in os.walk(args.dir):
        dirs[:] = [d for d in dirs if d+'/' not in args.exclude]
        for f in files:
            if f[-4:] in args.extensions:
                to_check.append(root+'/'+f)

    pattern = re.compile(r'RegisterPlot<TH[12][CLIFDS]>\s*\(\s*"([^"]+)"')

    exit_code = 0
    for file in to_check:
        matches = defaultdict(list)
        with open(file,'r') as f:
            for lineno, line in enumerate(f, start=1):
                m = pattern.search(line)
                if m:
                    first_arg = m.group(1).strip()
                    matches[first_arg].append(f"{file}:{lineno}")
        for key,value in matches.items():
            if len(value) > 1:
                exit_code = 1
                for loc in value:
                    print(f'{key} defined in multiple locations including here {loc}',file=sys.stderr)
    sys.exit(exit_code)
