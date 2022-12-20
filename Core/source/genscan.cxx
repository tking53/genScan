#include "genscan.h"
#include <getopt.h>

int main(int argc, char *argv[]) {

    struct option longOpts[] = {
            {"config",         required_argument, NULL, 'c'},
            {"outputfile",     required_argument, NULL, 'o'},
            {"inputfile",      required_argument, NULL, 'i'},
            {"evtbuild",       no_argument,       NULL, 'e'},
            {NULL,            no_argument,       NULL, 0} 
    };

    genscan scan;

    int retval = 0;
    int idx = 0;
 //getopt_long is not POSIX compliant. It is provided by GNU. This may mean
    //that we are not compatable with some systems. If we have enough
    //complaints we can either change it to getopt, or implement our own class.
    while ((retval = getopt_long(argc, argv, "coie", longOpts, &idx)) !=
           -1) {
        switch (retval) {
            case 'c':
                if (optarg != 0) {
                    scan.SetCfgFile(optarg);
                }
                break;
            default:
                break;
        };
    }

return 0;
}
