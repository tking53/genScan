#ifndef GENSCAN_HPP
#define GENSCAN_HPP

#include <iostream>
#include <thread>
#include <utility>
#include <map>
#include <string.h>

#include <sys/stat.h> //For directory manipulation

using namespace std;

class genscan{
    public:
        genscan() {};

        ~genscan() = default;


        const double i = 10;

    void SetCfgFile(string a){
        cfgFile = a;
    }

    private:
        string cfgFile;
        

};
#endif
