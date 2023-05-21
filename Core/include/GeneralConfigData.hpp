#ifndef GENERAL_CONFIG_DATA_HPP
#define GENERAL_CONFIG_DATA_HPP

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#include <cmath>
#include <cstdlib>
#include <stdint.h>

using namespace std;

class GeneralConfigData{

    public: 
    ///@return only instance of GeneralConfigData class.
    static GeneralConfigData *get();

    ///@return only instance of GeneralConfigData class.
    static GeneralConfigData *get(const string &file);

    ///The default destructor
    ~GeneralConfigData();

    ///Method that appends the output path to the provided string.
    ///@param[in] a : The string that we want to append to the output path. Most often this is going to be a filename
    /// of some sort.
    ///@return The concatenation of the provided string and the Output Path
    string AppendOutputPath(const string &a) { return outputPath_ + a; }

    ///@param[in] freq : The frequency of the module
    ///@return the correct adc clock conversion factor for the given freq
    double GetAdcClockInSeconds(const int &freq) const {
        if (XIAadcClockTickToSeconds_.find(freq) != XIAadcClockTickToSeconds_.end()) {
            return (XIAadcClockTickToSeconds_.find(freq)->second);
        } else {
            cout << "ERROR:: GeneralConfigData::GetAdcClockInSeconds(): Unknown Frequency, using Revision Default" << endl;
            return -1;
        }
    }

    ///@param[in] freq : The frequency of the module
    ///@return the correct clock conversion factor for the given freq
    double GetClockInSeconds(const int &freq) const {
        if (XIAclockTickToSeconds_.find(freq) != XIAclockTickToSeconds_.end()) {
            return (XIAclockTickToSeconds_.find(freq)->second);
        } else {
            cout << "ERROR:: GeneralConfigData::GetClockInSeconds(): Unknown Frequency, using Revision Default" << endl;
            return -1;
        }
    }

    ///@return the configuration file
    string GetConfigFileName() const { return configFile_; }

    ///@return the event size in seconds
    double GetEventLength() const { return eventLength_; }

    ///@param[in] freq : The frequency of the module
    ///@return the correct filter clock conversion factor for the given freq
    double GetFilterClockInSeconds(const int &freq) const {
        if (XIAfilterClockTickToSeconds_.find(freq) != XIAfilterClockTickToSeconds_.end()) {
            return (XIAfilterClockTickToSeconds_.find(freq)->second);
        } else {
            cout << "ERROR:: GeneralConfigData::GetFilterClockInSeconds(): Unknown Frequency, using Revision Default" << endl;
            return -1;
        }
    }

    ///@return returns name of specified output file
    string GetOutputFileName() const { return outputFilename_; }

    ///@return Path where additional files will be output.
    string GetOutputPath() { return outputPath_; }

    /// set digitizer type
    void SetDigitizer(const string &a) {digitizer_ = a;}

    ///Sets output Filename from scan interface
    ///@param[in] a : The parameter that we are going to set
    void SetOutputFilename(const string &a) { outputFilename_ = a; }

    ///Sets the path that we are going to output all of the files to.
    ///@param[in] a : The parameter that we are going to set
    void SetOutputPath(const string &a) { outputPath_ = a; }

    ///Set the event length in seconds. Different from pixie16:PAASS
    /// we will do all event building in time space from the outset
    ///@param[in] a : Event Length in seconds that we will set
    void SetEventLength(const double &a) {eventLength_ = a; }


private:
    ///Default Constructor 
    GeneralConfigData();

    ///The default constructor
    GeneralConfigData(GeneralConfigData const &);

    ///Copy constructor
    void operator=(GeneralConfigData const &);

    ///The one and only one instance of the class.
    static GeneralConfigData *instance_;

    ///A method that simply initializes all of the member varaiables to some
    /// default values. This will prevent too many errors down the line if
    /// they are not set properly due to invalid up configuration files.
    void InitializeMemberVariables(void);

    const map<int, double> XIAclockTickToSeconds_ = {{100, 10e-9}, {250, 8e-9}, {500, 10e-9}};        //!< map of frequencies and conversion factors
    const map<int, double> XIAadcClockTickToSeconds_ = {{100, 10e-9}, {250, 4e-9}, {500, 2e-9}};      //!< map of frequencies and conversion factors for Adc Ticks->Seconds
    const map<int, double> XIAfilterClockTickToSeconds_ = {{100, 10e-9}, {250, 8e-9}, {500, 10e-9}};  //!< map of frequencies and conversion factors for Dsp Ticks->Seconds
    string configFile_;                                     //!< The configuration file
    double eventLength_;                                //!< event width in seconds
    bool hasRawHistogramsDefined_;                               //!< True if we are plotting Raw Histograms
    string outputFilename_;                                 //!<Output Filename
    string outputPath_;                                     //!< The path to additional configuration files
    string digitizer_;                                       //!< the digitizer type
    double sysClockFreqInHz_;                                    //!< frequency of the system clock
    vector<pair<unsigned int, unsigned int>> reject_;  ///< Rejection regions
    double vandleBigSpeedOfLight_;                               //!< speed of light in big VANDLE bars in cm/ns
    double vandleMediumSpeedOfLight_;                            //!< speed of light in medium VANDLE bars in cm/ns
    double vandleSmallSpeedOfLight_;                             //!< speed of light in small VANDLE bars in cm/ns
};

#endif  // #ifdef GeneralConfigData_HPP_


