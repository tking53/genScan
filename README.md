# genScan

## Required Dependencies
- fmt
- spdlog
- pugixml
- yaml-cpp
- boost
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

Currently we require a version of root that has RVec and RDataFrame.
RVec is required for compilation of the struct libraries.
RDataFrame is required for scripts shipped.

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
|yaml-cpp-0.6.3   | No  | 
|yaml-cpp-0.8.0   | Yes | 

### boost
boost needs to be installed externally (usually via package)

MacOS
```brew install boost```

Ubuntu/Debian 
```apt install libboost-all-dev```

If one does not wish to install it externally, we will download a compatible version from 
Below is the current compatibility list, if you need a specific one tested let us know.

|version | Compatibility |
|:-------|:-------------:|
|libboost-1.89.0   | Yes  | 

## Usage and Generated Executables 
All compiled executables are prefixed with Gen, (i.e. GenScanor, GenPeakFit, etc.)
There are also several scripts in both python and root that are shipped as well.
The python scripts contain no extension and are mostly used for automated calibration and gain matching.
The root scripts do have their extensions (.cxx) when installed.
The root scripts are self contained and use standard C++ and root features.

### Output Files
In addition to the root file output, four more files are created.
A list file containing the names, sizes, titles, and bounds of each histogram.
A log file containing information of all items tagged with [info] and [critical]
A dbg file containing information of all items tagged with [info], [critical], [error], and [debug] 
A err file containing information of all items tagged with [critical] and [error]

Also contained in the dbg file is the parsing information of the channel map, as well
as accumulated pileup, saturation, and hit statistics for each channel as it was decoded.

### Currently supported file formats
- LDF (UTK Pixie16)
- EVT (NSCLDaq Pixie16)
- EVT_TO (Allmond Pixie16)
