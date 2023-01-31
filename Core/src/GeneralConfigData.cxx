#include "GeneralConfigData.hpp"

using namespace std;

/// Initializes the instance_ of the class to a null pointer.
GeneralConfigData *GeneralConfigData::instance_ = nullptr;

// Instance is created upon first call
GeneralConfigData* GeneralConfigData::get() {
    if (!instance_) {
        cout << "ERROR CFG not parsed yet" << endl;
    }
    return (instance_);
}

/// The destructor that will delete the instance_ of the class.
GeneralConfigData::~GeneralConfigData() {
    delete (instance_);
    instance_ = nullptr;
}
