# genScan

## Required Dependencies
- fmt
- spdlog
- pugixml
- jsoncpp
- yaml-cpp
- ROOT

### ROOT 
CERN's root is required, typically it is best to build this from source yourself
The root dependencies can be found here https://root.cern/install/dependencies/
While the release packages can be found here https://root.cern/install/all_releases/

If the release packages site is down then one can download everything from the public repo found here https://github.com/root-project/root
If you're using the github repo method, the stable releases are all tagged and can be easily retrieved by first cloning the repo
```git clone https://github.com/root-project/root```

cd into the newly created directory
Then checkout the particular tag you want
```git checkout tags/<tag_name>```

Where <tag_name> is the particular version you want (i.e. v6-28-06)


### spdlog
spdlog needs to be installed externally (usually via package)

MacOS 
```brew install spdlog```

Ubuntu/Debian
``` apt install libspdlog-dev```

If one does not wish to install it externally, we will download a compatible version from https://github.com/gabime/spdlog 
Below is the current compatibility list, if you need a specific one tested let us know.

|version | Compatibility |
|:-------|:-------------:|
|v1.9.2   | Yes | 

### fmt
fmt needs to be installed externally (usually via package)

MacOS 
```brew install fmt```

Ubuntu/Debian
``` apt install libfmt-dev```

If one does not wish to install it externally, we will download a compatible version from https://github.com/fmtlib/fmt
Below is the current compatibility list, if you need a specific one tested let us know.

|version | Compatibility |
|:-------|:-------------:|
|v8.1.1   | Yes | 

### pugixml
pugixml needs to be installed externally (usually via package)

MacOS 
```brew install pugixml```

Ubuntu/Debian
``` apt install libpugixml-dev```

If one does not wish to install it externally, we will download a compatible version from https://github.com/zeux/pugixml
Below is the current compatibility list, if you need a specific one tested let us know.

|version | Compatibility |
|:-------|:-------------:|
|v1.13   | Yes | 

### jsoncpp
jsoncpp needs to be installed externally (usually via package)

MacOS 
```brew install jsoncpp```

Ubuntu/Debian
``` apt install libjsoncpp-dev```

If one does not wish to install it externally, we will download a compatible version from https://github.com/open-source-parsers/jsoncpp 
Below is the current compatibility list, if you need a specific one tested let us know.

|version | Compatibility |
|:-------|:-------------:|
|1.9.5   | Yes | 

### yaml-cpp
yaml-cpp needs to be installed externally (usually via package)

MacOS 
```brew install yaml-cpp```

Ubuntu/Debian
``` apt install libyaml-cpp-dev```

If one does not wish to install it externally, we will download a compatible version from https://github.com/jbeder/yaml-cpp 
Below is the current compatibility list, if you need a specific one tested let us know.

|version | Compatibility |
|:-------|:-------------:|
|yaml-cpp-0.6.3   | Yes | 

## Usage (GenScanor)
Providing GenScanor with no arguments will have it print the help message

### Currently supported options
``` 
./GenScanor \
-c/--configfile [filename] filename for channel map \
-o/--outputfile [filename] filename for output \ 
-e/--evtbuild event build only \
-f/--filenames [file1 file2 file3 ...] list of files used for input \
-h/--help show this message \
-l/--limit limit of coincidence queue \
-x/--fileformat [file_format] format of the data file (evt,ldf,pld,caen_root,caen_bin) \
```

### Output Files
In addition to the root file output, three more files are created.

- genscan.log this is a log and contains information about when things occur during the program runtime a sample is shown below
```
INSERT EXAMPLE OF LOG
```

- genscan.dbg this contains the same information as genscan.log, but also contains debugging information for when things go awry, a sample is show below
```
INSERT EXAMPLE OF DEBUG
```

- genscan.err this contains any errors that occured during runtime and should be empty in almost all use cases, but an example is shown below
```
INSERT EXAMPLE OF ERROR
```

The output root file contains the correlated events stored as 
```
INSERT FORMAT HERE
```

### Currently supported file formats
- PLD (UTK Pixie16)
- LDF (UTK Pixie16)
- CAEN (ROOT)
- CAEN (Single Binary File)
- CAEN (Multiple Binary File)
- EVT (NSCLDaq)
