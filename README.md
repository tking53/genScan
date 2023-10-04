# genScan

## Required Dependencies
-spdlog
-pugixml
-ROOT

## ROOT 
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


## spdlog
easily build and installed from https://github.com/gabime/spdlog
This does force one to use a C++17 compliant compiler due to conflicts with ROOT in string_view

##pugixml
pugixml needs to be installed externally (usually via package)

MacOS 
```brew install pugixml```

Ubuntu/Debian
``` apt install libpugixml-dev```


