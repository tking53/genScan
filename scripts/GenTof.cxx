#include <string>
#include <vector>
#include <map>
#include <utility>
#include <random>

#include <TFile.h>
#include <TTree.h>
#include <RDataFrame.hxx>
#include <RVec.hxx>
#include <ROOT/RDF/HistoModels.hxx>
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RResultHandle.hxx>
#include <RSnapshotOptions.hxx>
#include <TMath.h>

#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#include "DeuteratedLiquidScintStruct.hpp"
#include "RootDevStruct.hpp"

constexpr auto group_640_upper_edges = {
	1.0000E-10, 1.0500E-10, 1.1000E-10, 1.1500E-10, 1.2000E-10, 
	1.2750E-10, 1.3500E-10, 1.4250E-10, 1.5000E-10, 1.6000E-10, 
	1.7000E-10, 1.8000E-10, 1.9000E-10, 2.0000E-10, 2.1000E-10, 
	2.2000E-10, 2.3000E-10, 2.4000E-10, 2.5500E-10, 2.7000E-10, 
	2.8000E-10, 3.0000E-10, 3.2000E-10, 3.4000E-10, 3.6000E-10, 
	3.8000E-10, 4.0000E-10, 4.2500E-10, 4.5000E-10, 4.7500E-10, 
	5.0000E-10, 5.2500E-10, 5.5000E-10, 5.7500E-10, 6.0000E-10, 
	6.3000E-10, 6.6000E-10, 6.9000E-10, 7.2000E-10, 7.6000E-10, 
	8.0000E-10, 8.4000E-10, 8.8000E-10, 9.2000E-10, 9.6000E-10, 
	1.0000E-09, 1.0500E-09, 1.1000E-09, 1.1500E-09, 1.2000E-09, 
	1.2750E-09, 1.3500E-09, 1.4250E-09, 1.5000E-09, 1.6000E-09, 
	1.7000E-09, 1.8000E-09, 1.9000E-09, 2.0000E-09, 2.1000E-09, 
	2.2000E-09, 2.3000E-09, 2.4000E-09, 2.5500E-09, 2.7000E-09, 
	2.8000E-09, 3.0000E-09, 3.2000E-09, 3.4000E-09, 3.6000E-09, 
	3.8000E-09, 4.0000E-09, 4.2500E-09, 4.5000E-09, 4.7500E-09, 
	5.0000E-09, 5.2500E-09, 5.5000E-09, 5.7500E-09, 6.0000E-09, 
	6.3000E-09, 6.6000E-09, 6.9000E-09, 7.2000E-09, 7.6000E-09, 
	8.0000E-09, 8.4000E-09, 8.8000E-09, 9.2000E-09, 9.6000E-09, 
	1.0000E-08, 1.0500E-08, 1.1000E-08, 1.1500E-08, 1.2000E-08, 
	1.2750E-08, 1.3500E-08, 1.4250E-08, 1.5000E-08, 1.6000E-08, 
	1.7000E-08, 1.8000E-08, 1.9000E-08, 2.0000E-08, 2.1000E-08, 
	2.2000E-08, 2.3000E-08, 2.4000E-08, 2.5500E-08, 2.7000E-08, 
	2.8000E-08, 3.0000E-08, 3.2000E-08, 3.4000E-08, 3.6000E-08, 
	3.8000E-08, 4.0000E-08, 4.2500E-08, 4.5000E-08, 4.7500E-08, 
	5.0000E-08, 5.2500E-08, 5.5000E-08, 5.7500E-08, 6.0000E-08, 
	6.3000E-08, 6.6000E-08, 6.9000E-08, 7.2000E-08, 7.6000E-08, 
	8.0000E-08, 8.4000E-08, 8.8000E-08, 9.2000E-08, 9.6000E-08, 
	1.0000E-07, 1.0500E-07, 1.1000E-07, 1.1500E-07, 1.2000E-07, 
	1.2750E-07, 1.3500E-07, 1.4250E-07, 1.5000E-07, 1.6000E-07, 
	1.7000E-07, 1.8000E-07, 1.9000E-07, 2.0000E-07, 2.1000E-07, 
	2.2000E-07, 2.3000E-07, 2.4000E-07, 2.5500E-07, 2.7000E-07, 
	2.8000E-07, 3.0000E-07, 3.2000E-07, 3.4000E-07, 3.6000E-07, 
	3.8000E-07, 4.0000E-07, 4.2500E-07, 4.5000E-07, 4.7500E-07, 
	5.0000E-07, 5.2500E-07, 5.5000E-07, 5.7500E-07, 6.0000E-07, 
	6.3000E-07, 6.6000E-07, 6.9000E-07, 7.2000E-07, 7.6000E-07, 
	8.0000E-07, 8.4000E-07, 8.8000E-07, 9.2000E-07, 9.6000E-07, 
	1.0000E-06, 1.0500E-06, 1.1000E-06, 1.1500E-06, 1.2000E-06, 
	1.2750E-06, 1.3500E-06, 1.4250E-06, 1.5000E-06, 1.6000E-06, 
	1.7000E-06, 1.8000E-06, 1.9000E-06, 2.0000E-06, 2.1000E-06, 
	2.2000E-06, 2.3000E-06, 2.4000E-06, 2.5500E-06, 2.7000E-06, 
	2.8000E-06, 3.0000E-06, 3.2000E-06, 3.4000E-06, 3.6000E-06, 
	3.8000E-06, 4.0000E-06, 4.2500E-06, 4.5000E-06, 4.7500E-06, 
	5.0000E-06, 5.2500E-06, 5.5000E-06, 5.7500E-06, 6.0000E-06, 
	6.3000E-06, 6.6000E-06, 6.9000E-06, 7.2000E-06, 7.6000E-06, 
	8.0000E-06, 8.4000E-06, 8.8000E-06, 9.2000E-06, 9.6000E-06, 
	1.0000E-05, 1.0500E-05, 1.1000E-05, 1.1500E-05, 1.2000E-05, 
	1.2750E-05, 1.3500E-05, 1.4250E-05, 1.5000E-05, 1.6000E-05, 
	1.7000E-05, 1.8000E-05, 1.9000E-05, 2.0000E-05, 2.1000E-05, 
	2.2000E-05, 2.3000E-05, 2.4000E-05, 2.5500E-05, 2.7000E-05, 
	2.8000E-05, 3.0000E-05, 3.2000E-05, 3.4000E-05, 3.6000E-05, 
	3.8000E-05, 4.0000E-05, 4.2500E-05, 4.5000E-05, 4.7500E-05, 
	5.0000E-05, 5.2500E-05, 5.5000E-05, 5.7500E-05, 6.0000E-05, 
	6.3000E-05, 6.6000E-05, 6.9000E-05, 7.2000E-05, 7.6000E-05, 
	8.0000E-05, 8.4000E-05, 8.8000E-05, 9.2000E-05, 9.6000E-05, 
	1.0000E-04, 1.0500E-04, 1.1000E-04, 1.1500E-04, 1.2000E-04, 
	1.2750E-04, 1.3500E-04, 1.4250E-04, 1.5000E-04, 1.6000E-04, 
	1.7000E-04, 1.8000E-04, 1.9000E-04, 2.0000E-04, 2.1000E-04, 
	2.2000E-04, 2.3000E-04, 2.4000E-04, 2.5500E-04, 2.7000E-04, 
	2.8000E-04, 3.0000E-04, 3.2000E-04, 3.4000E-04, 3.6000E-04, 
	3.8000E-04, 4.0000E-04, 4.2500E-04, 4.5000E-04, 4.7500E-04, 
	5.0000E-04, 5.2500E-04, 5.5000E-04, 5.7500E-04, 6.0000E-04, 
	6.3000E-04, 6.6000E-04, 6.9000E-04, 7.2000E-04, 7.6000E-04, 
	8.0000E-04, 8.4000E-04, 8.8000E-04, 9.2000E-04, 9.6000E-04, 
	1.0000E-03, 1.0500E-03, 1.1000E-03, 1.1500E-03, 1.2000E-03, 
	1.2750E-03, 1.3500E-03, 1.4250E-03, 1.5000E-03, 1.6000E-03, 
	1.7000E-03, 1.8000E-03, 1.9000E-03, 2.0000E-03, 2.1000E-03, 
	2.2000E-03, 2.3000E-03, 2.4000E-03, 2.5500E-03, 2.7000E-03, 
	2.8000E-03, 3.0000E-03, 3.2000E-03, 3.4000E-03, 3.6000E-03, 
	3.8000E-03, 4.0000E-03, 4.2500E-03, 4.5000E-03, 4.7500E-03, 
	5.0000E-03, 5.2500E-03, 5.5000E-03, 5.7500E-03, 6.0000E-03, 
	6.3000E-03, 6.6000E-03, 6.9000E-03, 7.2000E-03, 7.6000E-03, 
	8.0000E-03, 8.4000E-03, 8.8000E-03, 9.2000E-03, 9.6000E-03, 
	1.0000E-02, 1.0500E-02, 1.1000E-02, 1.1500E-02, 1.2000E-02, 
	1.2750E-02, 1.3500E-02, 1.4250E-02, 1.5000E-02, 1.6000E-02, 
	1.7000E-02, 1.8000E-02, 1.9000E-02, 2.0000E-02, 2.1000E-02, 
	2.2000E-02, 2.3000E-02, 2.4000E-02, 2.5500E-02, 2.7000E-02, 
	2.8000E-02, 3.0000E-02, 3.2000E-02, 3.4000E-02, 3.6000E-02, 
	3.8000E-02, 4.0000E-02, 4.2500E-02, 4.5000E-02, 4.7500E-02, 
	5.0000E-02, 5.2500E-02, 5.5000E-02, 5.7500E-02, 6.0000E-02, 
	6.3000E-02, 6.6000E-02, 6.9000E-02, 7.2000E-02, 7.6000E-02, 
	8.0000E-02, 8.4000E-02, 8.8000E-02, 9.2000E-02, 9.6000E-02, 
	1.0000E-01, 1.0500E-01, 1.1000E-01, 1.1500E-01, 1.2000E-01, 
	1.2750E-01, 1.3500E-01, 1.4250E-01, 1.5000E-01, 1.6000E-01, 
	1.7000E-01, 1.8000E-01, 1.9000E-01, 2.0000E-01, 2.1000E-01, 
	2.2000E-01, 2.3000E-01, 2.4000E-01, 2.5500E-01, 2.7000E-01, 
	2.8000E-01, 3.0000E-01, 3.2000E-01, 3.4000E-01, 3.6000E-01, 
	3.8000E-01, 4.0000E-01, 4.2500E-01, 4.5000E-01, 4.7500E-01, 
	5.0000E-01, 5.2500E-01, 5.5000E-01, 5.7500E-01, 6.0000E-01, 
	6.3000E-01, 6.6000E-01, 6.9000E-01, 7.2000E-01, 7.6000E-01, 
	8.0000E-01, 8.4000E-01, 8.8000E-01, 9.2000E-01, 9.6000E-01, 
	1.0000E+00, 1.1000E+00, 1.2000E+00, 1.3000E+00, 1.4000E+00, 
	1.5000E+00, 1.6000E+00, 1.7000E+00, 1.8000E+00, 1.9000E+00, 
	2.0000E+00, 2.1000E+00, 2.2000E+00, 2.3000E+00, 2.4000E+00, 
	2.5000E+00, 2.6000E+00, 2.7000E+00, 2.8000E+00, 2.9000E+00, 
	3.0000E+00, 3.1000E+00, 3.2000E+00, 3.3000E+00, 3.4000E+00, 
	3.5000E+00, 3.6000E+00, 3.7000E+00, 3.8000E+00, 3.9000E+00, 
	4.0000E+00, 4.1000E+00, 4.2000E+00, 4.3000E+00, 4.4000E+00, 
	4.5000E+00, 4.6000E+00, 4.7000E+00, 4.8000E+00, 4.9000E+00, 
	5.0000E+00, 5.1000E+00, 5.2000E+00, 5.3000E+00, 5.4000E+00, 
	5.5000E+00, 5.6000E+00, 5.7000E+00, 5.8000E+00, 5.9000E+00, 
	6.0000E+00, 6.1000E+00, 6.2000E+00, 6.3000E+00, 6.4000E+00, 
	6.5000E+00, 6.6000E+00, 6.7000E+00, 6.8000E+00, 6.9000E+00, 
	7.0000E+00, 7.1000E+00, 7.2000E+00, 7.3000E+00, 7.4000E+00, 
	7.5000E+00, 7.6000E+00, 7.7000E+00, 7.8000E+00, 7.9000E+00, 
	8.0000E+00, 8.1000E+00, 8.2000E+00, 8.3000E+00, 8.4000E+00, 
	8.5000E+00, 8.6000E+00, 8.7000E+00, 8.8000E+00, 8.9000E+00, 
	9.0000E+00, 9.1000E+00, 9.2000E+00, 9.3000E+00, 9.4000E+00, 
	9.5000E+00, 9.6000E+00, 9.7000E+00, 9.8000E+00, 9.9000E+00, 
	1.0000E+01, 1.0100E+01, 1.0200E+01, 1.0300E+01, 1.0400E+01, 
	1.0500E+01, 1.0600E+01, 1.0700E+01, 1.0800E+01, 1.0900E+01, 
	1.1000E+01, 1.1100E+01, 1.1200E+01, 1.1300E+01, 1.1400E+01, 
	1.1500E+01, 1.1600E+01, 1.1700E+01, 1.1800E+01, 1.1900E+01, 
	1.2000E+01, 1.2100E+01, 1.2200E+01, 1.2300E+01, 1.2400E+01, 
	1.2500E+01, 1.2600E+01, 1.2700E+01, 1.2800E+01, 1.2900E+01, 
	1.3000E+01, 1.3100E+01, 1.3200E+01, 1.3300E+01, 1.3400E+01, 
	1.3500E+01, 1.3600E+01, 1.3700E+01, 1.3800E+01, 1.3900E+01, 
	1.4000E+01, 1.4100E+01, 1.4200E+01, 1.4300E+01, 1.4400E+01, 
	1.4500E+01, 1.4600E+01, 1.4700E+01, 1.4800E+01, 1.4900E+01, 
	1.5000E+01, 1.5100E+01, 1.5200E+01, 1.5300E+01, 1.5400E+01, 
	1.5500E+01, 1.5600E+01, 1.5700E+01, 1.5800E+01, 1.5900E+01, 
	1.6000E+01, 1.6100E+01, 1.6200E+01, 1.6300E+01, 1.6400E+01, 
	1.6500E+01, 1.6600E+01, 1.6700E+01, 1.6800E+01, 1.6900E+01, 
	1.7000E+01, 1.7100E+01, 1.7200E+01, 1.7300E+01, 1.7400E+01, 
	1.7500E+01, 1.7600E+01, 1.7700E+01, 1.7800E+01, 1.7900E+01, 
	1.8000E+01, 1.8100E+01, 1.8200E+01, 1.8300E+01, 1.8400E+01, 
	1.8500E+01, 1.8600E+01, 1.8700E+01, 1.8800E+01, 1.8900E+01, 
	1.9000E+01, 1.9100E+01, 1.9200E+01, 1.9300E+01, 1.9400E+01, 
	1.9500E+01, 1.9600E+01, 1.9700E+01, 1.9800E+01, 1.9900E+01, 
	2.0000E+01,
};

constexpr auto group_129_upper_edges = {
	1.0000E-10, 1.0000E-09, 1.0000E-08, 2.3000E-08, 5.0000E-08,
	7.6000E-08, 1.1500E-07, 1.7000E-07, 2.5500E-07, 3.8000E-07,
	5.5000E-07, 8.4000E-07, 1.2800E-06, 1.9000E-06, 2.8000E-06,
	4.2500E-06, 6.3000E-06, 9.2000E-06, 1.3500E-05, 2.1000E-05,
	3.0000E-05, 4.5000E-05, 6.9000E-05, 1.0000E-04, 1.3500E-04,
	1.7000E-04, 2.2000E-04, 2.8000E-04, 3.6000E-04, 4.5000E-04,
	5.7500E-04, 7.6000E-04, 9.6000E-04, 1.2800E-03, 1.6000E-03,
	2.0000E-03, 2.7000E-03, 3.4000E-03, 4.5000E-03, 5.5000E-03,
	7.2000E-03, 9.2000E-03, 1.2000E-02, 1.5000E-02, 1.9000E-02,
	2.5500E-02, 3.2000E-02, 4.0000E-02, 5.2500E-02, 6.6000E-02,
	8.8000E-02, 1.1000E-01, 1.3500E-01, 1.6000E-01, 1.9000E-01,
	2.2000E-01, 2.5500E-01, 2.9000E-01, 3.2000E-01, 3.6000E-01,
	4.0000E-01, 4.5000E-01, 5.0000E-01, 5.5000E-01, 6.0000E-01,
	6.6000E-01, 7.2000E-01, 7.8000E-01, 8.4000E-01, 9.2000E-01,
	1.0000E+00, 1.2000E+00, 1.4000E+00, 1.6000E+00, 1.8000E+00,
	2.0000E+00, 2.3000E+00, 2.6000E+00, 2.9000E+00, 3.3000E+00,
	3.7000E+00, 4.1000E+00, 4.5000E+00, 5.0000E+00, 5.5000E+00,
	6.0000E+00, 6.7000E+00, 7.4000E+00, 8.2000E+00, 9.0000E+00,
	1.0000E+01, 1.1000E+01, 1.2000E+01, 1.2500E+01, 1.2600E+01,
	1.2700E+01, 1.2800E+01, 1.2900E+01, 1.3000E+01, 1.3100E+01,
	1.3200E+01, 1.3300E+01, 1.3400E+01, 1.3500E+01, 1.3600E+01,
	1.3700E+01, 1.3800E+01, 1.3900E+01, 1.4000E+01, 1.4100E+01,
	1.4200E+01, 1.4300E+01, 1.4400E+01, 1.4500E+01, 1.4600E+01,
	1.4700E+01, 1.4800E+01, 1.4900E+01, 1.5000E+01, 1.5100E+01,
	1.5200E+01, 1.5300E+01, 1.5400E+01, 1.5500E+01, 1.5600E+01,
	1.5700E+01, 1.5800E+01, 1.5900E+01, 1.6000E+01, 1.6500E+01,
};
// clang-format on

template<class T>
class HisHelper {
	private:
		T model;

	public:
		HisHelper(const std::string& name, const std::string& title, const int& nbinsx, const double& xlow, const double& xup)
			: model(name.c_str(), title.c_str(), nbinsx, xlow, xup) {
			}

		HisHelper(const std::string& name, const std::string& title, const int& nbinsx, const double& xlow, const double& xup, const int& nbinsy, const double& ylow, const double& yup)
			: model(name.c_str(), title.c_str(), nbinsx, xlow, xup, nbinsy, ylow, yup) {
			}

		template<class U>
			HisHelper(const std::string& name, const std::string& title, const U& xedges)
			: model(name.c_str(), title.c_str(), xedges.size() - 1, xedges.data()) {
			}

		template<class U>
			HisHelper(const std::string& name, const std::string& title, const U& xedges, const U& yedges)
			: model(name.c_str(), title.c_str(), xedges.size() - 1, xedges.data(), yedges.size() - 1, yedges.data()) {
			}

		template<class U>
			HisHelper(const std::string& name, const std::string& title, const int& nbinsx, const double& xlow, const double& xup, const U& yedges)
			: model(name.c_str(), title.c_str(), nbinsx, xlow, xup, yedges.size() - 1, yedges.data()) {
			}

		template<class U>
			HisHelper(const std::string& name, const std::string& title, const U& xedges, const int& nbinsy, const double& ylow, const double& yup)
			: model(name.c_str(), title.c_str(), xedges.size() - 1, xedges.data(), nbinsy, ylow, yup) {
			}

		HisHelper(const HisHelper&) = default;
		HisHelper(HisHelper&&) = default;
		HisHelper& operator=(const HisHelper&) = default;
		HisHelper& operator=(HisHelper&&) = default;
		~HisHelper() = default;

		T GetModel() const {
			return this->model;
		}

		void ResetModel(const std::string& name, const std::string& title, const int& nbinsx, const double& xlow, const double& xup) {
			this->model = T(name.c_str(), title.c_str(), nbinsx, xlow, xup);
		}

		void ResetModel(const std::string& name, const std::string& title, const int& nbinsx, const double& xlow, const double& xup, const int& nbinsy, const double& ylow, const double& yup) {
			this->model = T(name.c_str(), title.c_str(), nbinsx, xlow, xup, nbinsy, ylow, yup);
		}

		template<class U>
			void ResetModel(const std::string& name, const std::string& title, const U& xedges) {
				this->model = T(name.c_str(), title.c_str(), xedges.size() - 1, xedges.data());
			}

		template<class U>
			void ResetModel(const std::string& name, const std::string& title, const U& xedges, const U& yedges) {
				this->model = T(name.c_str(), title.c_str(), xedges.size() - 1, xedges.data(), yedges.size() - 1, yedges.data());
			}

		template<class U>
			void ResetModel(const std::string& name, const std::string& title, const int& nbinsx, const double& xlow, const double& xup, const U& yedges) {
				this->model = T(name.c_str(), title.c_str(), nbinsx, xlow, xup, yedges.size() - 1, yedges.data());
			}

		template<class U>
			void ResetModel(const std::string& name, const std::string& title, const U& xedges, const int& nbinsy, const double& ylow, const double& yup) {
				this->model = T(name.c_str(), title.c_str(), xedges.size() - 1, xedges.data(), nbinsy, ylow, yup);
			}
};

using h1_info = std::tuple<HisHelper<ROOT::RDF::TH1DModel>, std::string>;
using h2_info = std::tuple<HisHelper<ROOT::RDF::TH2DModel>, std::string, std::string>;
using h1d_model = std::tuple<ROOT::RDF::TH1DModel, std::string>;
using h2d_model = std::tuple<ROOT::RDF::TH2DModel, std::string, std::string>;

using RVecDLS = ROOT::RVec<ProcessorStruct::DLS>;
using RVecRootDev = ROOT::RVec<ProcessorStruct::RootDev>;

void GenTof(const std::string& config,const std::string& inputfile,const std::string& outputprefix) {
	// inputs from config file
	
	YAML::Node doc = YAML::LoadFile(config);
	std::string outputfile = outputprefix;
	auto dist = doc["distance"].as<double>();
	auto delta_dist = doc["length"].as<double>();
	auto offset = doc["offset"].as<double>();
	auto psd_high = doc["psd"]["high"].as<double>();
	auto psd_low = doc["psd"]["low"].as<double>();
	auto light_low = doc["light"]["low"].as<double>();
	auto tdiff_high = doc["tdiff"]["high"].as<double>();
	auto tdiff_low = doc["tdiff"]["low"].as<double>();
	// std::string inputfile = "/Users/truland/run3merged.root";
	// std::string outputfile = "/Users/truland/run3response.root";
	// double dist = 7.86; // m 
	// double offset = 46.596; // 46.4914 for run 2, 45.8008 for run 3
	// double psd_low = 0.15;
	// double psd_high = 0.5;
	// double light_low = 100.0;
	// double tdiff_low = 95.0;
	// double tdiff_high = 988.0;
	
	// constants and fixed values no matter what
	std::string neutrontree = "DeutLS";
	std::string beamtree = "RootDev";
	double mass = 939.56542194; // MeV / c^2
	double c = 0.299792458; // m / ns
			
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> uniform(-delta_dist,delta_dist);

	std::map<std::string,h1_info> h1_singles_map {
		{"tdiff",{HisHelper<ROOT::RDF::TH1DModel>("tdiff","tdiff Neutron - #gamma; Time Difference (ns)",3000,-1500,1500),"tdiff"}},
	};
	std::map<std::string,h1_info> h1_neutrons_map {
		{"beta",{HisHelper<ROOT::RDF::TH1DModel>("beta","#beta; Relativistic Velocity (arb.)",1000,0,1.0),"beta"}},
		{"gamma",{HisHelper<ROOT::RDF::TH1DModel>("gamma","#gamma; Lorentz Factor (arb.)",1000,1,2),"gamma"}},
	};

	std::map<std::string,h2_info> h2_singles_map {
		{"bands",{HisHelper<ROOT::RDF::TH2DModel>("bands","bands; Light (keVee); PSD (arb.)",6000,0,6000,1000,0,1),"light","psd"}},
	};
	std::map<std::string,h2_info> h2_neutrons_map {
		{"response_classical",{HisHelper<ROOT::RDF::TH2DModel>("response_classical","response classical; Light (keVee); Energy (MeV);",6000,0,6000,12000,0,12),"light","nerg_classical"}},
		{"response_relativistic",{HisHelper<ROOT::RDF::TH2DModel>("response_relativistic","response relativistic; Light (keVee); Energy (MeV);",6000,0,6000,12000,0,12),"light","nerg_relativistic"}},
	};

	auto input = TFile::Open(inputfile.c_str(),"READ");
	auto DeutLS = input->Get<TTree>(neutrontree.c_str());
	auto RootDev = input->Get<TTree>(beamtree.c_str());

	DeutLS->AddFriend(RootDev);

	auto single_event = [](const RVecDLS& dls, const RVecRootDev& rd) {
		return dls.size() == 1 and rd.size() == 1;
	};

	auto light = [](const RVecDLS& dls) {
		return dls[0].energy;
	};

	auto psd = [](const RVecDLS& dls) {
		return dls[0].psd;
	};

	auto neutrons = [&](const double& light,const double& psd, const double& tdiff) {
		return light >= light_low and 
		       (psd < psd_high and psd > psd_low) and 
		       (tdiff < tdiff_high and tdiff > tdiff_low);
	};

	auto tdiff = [](const RVecDLS& dls, const RVecRootDev& rd) {
		return dls[0].ts - rd[0].time;
	};

	auto tof = [&](const double& tdiff) {
		return tdiff - offset;
	};	

	// let's do something stupid
	// let's alias the distance based on the half-lenfth of the detector
	// we could probably also add in an angle alias based on the radius 
	// and then solve for the new "distance" of interaction
	// Ideally this isn't uniformly sampled across the length it has an exponential 
	// modulation based on the length and the cross section but then we 
	// would need to know the energy apriori, but if we assume our interaction volume 
	// is much less than the path length across the entire energy range, then we 
	// can assume it's a flat bias 
	auto distance = [&]() {
		return dist + uniform(gen);
	};

	auto beta = [&](const double& tof,const double& distr) {
		return distr / (c * tof);
	};

	auto gamma = [&](const double& beta) {
		return 1.0 / TMath::Sqrt(1.0 - beta * beta);
	};

	auto nerg_classical = [&](const double& tof,const double& distr) {
		return 0.5 * mass * (distr / tof) * (distr / tof) / (c * c);
	};

	auto nerg_relativistic = [&](const double& gamma) {
		return (gamma - 1.0) * mass;
	};

	auto df = ROOT::RDataFrame(*DeutLS); 
	auto df_filter = df.Filter(single_event,{"arr","data_vec"}) // Filter down to single events in each detector
			   .Define("light",light,{"arr"}) // Define light as an alias to help
			   .Define("psd",psd,{"arr"}) // Define psd as an alias to help
			   .Define("tdiff",tdiff,{"arr","data_vec"}); // Define the tdiff between the det and beam

	auto df_neutrons = df_filter.Filter(neutrons,{"light","psd","tdiff"}) // Filter out to just the neutrons
				    .Define("tof",tof,{"tdiff"}) // Define the tof for the neutrons
				    .Define("distance",distance,{}) // Define distance we will use
				    .Define("beta",beta,{"tof","distance"}) // Define beta helper to get the relativistic ToF
				    .Define("gamma",gamma,{"beta"}) // Define gamma helper to get the relativistic ToF
				    .Define("nerg_classical",nerg_classical,{"tof","distance"}) // Define the neutron energy classically
				    .Define("nerg_relativistic",nerg_relativistic,{"gamma"}); // Define the neutron energy relativistically

	ROOT::RDF::Experimental::AddProgressBar(df);

	auto get_model = [](const auto& info) {
		return std::get<0>(info).GetModel();
	};
	auto get_xvalue = [](const auto& info) {
		return std::get<1>(info);
	};
	auto get_yvalue = [](const auto& info) {
		return std::get<2>(info);
	};


	std::vector<ROOT::RDF::RResultHandle> plots;
	std::vector<size_t> axis;
	std::vector<h1d_model> models_1d;
	for (auto& kv : h1_singles_map) {
		auto model = get_model(kv.second);
		auto x = get_xvalue(kv.second);
		models_1d.push_back({model, x.c_str()});
	}
	std::vector<h2d_model> models_2d;
	for (auto& kv : h2_singles_map) {
		auto model = get_model(kv.second);
		auto x = get_xvalue(kv.second);
		auto y = get_yvalue(kv.second);
		models_2d.push_back({model, x.c_str(), y.c_str()});
	}
	for (const auto& h1 : models_1d) {
		plots.push_back(df_filter.Histo1D(std::get<0>(h1), std::get<1>(h1)));
		axis.push_back(1);
	}
	for (const auto& h2 : models_2d) {
		plots.push_back(df_filter.Histo2D(std::get<0>(h2), std::get<1>(h2), std::get<2>(h2)));
		axis.push_back(2);
	}

	// plots for neutron only filter
	std::vector<h1d_model> models_neutrons_1d;
	for (auto& kv : h1_neutrons_map) {
		auto model = get_model(kv.second);
		auto x = get_xvalue(kv.second);
		models_neutrons_1d.push_back({model, x.c_str()});
	}
	std::vector<h2d_model> models_neutrons_2d;
	for (auto& kv : h2_neutrons_map) {
		auto model = get_model(kv.second);
		auto x = get_xvalue(kv.second);
		auto y = get_yvalue(kv.second);
		models_neutrons_2d.push_back({model, x.c_str(), y.c_str()});
	}
	for (const auto& h1 : models_neutrons_1d) {
		plots.push_back(df_neutrons.Histo1D(std::get<0>(h1), std::get<1>(h1)));
		axis.push_back(1);
	}
	for (const auto& h2 : models_neutrons_2d) {
		plots.push_back(df_neutrons.Histo2D(std::get<0>(h2), std::get<1>(h2), std::get<2>(h2)));
		axis.push_back(2);
	}

	ROOT::RDF::RunGraphs(plots);

	std::filesystem::path outputpath(outputfile);
	outputpath.replace_extension(".root");
	outputfile = outputpath.string();

	auto ofile = new TFile(outputfile.c_str(), "RECREATE");
	std::cout << "Writing spectra" << std::endl;

	for (size_t ii = 0; ii < plots.size(); ++ii) {
		if (axis[ii] == 1) {
			auto plot = plots[ii].GetPtr<TH1D>();
			plot->SetDirectory(0);
			plot->GetXaxis()->SetTitleSize(0.04);
			plot->GetXaxis()->SetLabelSize(0.04);
			plot->GetYaxis()->SetTitleSize(0.04);
			plot->GetYaxis()->SetLabelSize(0.04);
			plot->GetXaxis()->CenterTitle();
			plot->GetYaxis()->CenterTitle();
			plot->Write(0, 2, 0);
		} else if (axis[ii] == 2) {
			auto plot = plots[ii].GetPtr<TH2D>();
			plot->SetDirectory(0);
			plot->GetXaxis()->SetTitleSize(0.04);
			plot->GetXaxis()->SetLabelSize(0.04);
			plot->GetYaxis()->SetTitleSize(0.04);
			plot->GetYaxis()->SetLabelSize(0.04);
			plot->GetXaxis()->CenterTitle();
			plot->GetYaxis()->CenterTitle();
			plot->Write(0, 2, 0);

		} else {
		}

	}
	ofile->Close();

	// std::cout << "Writing slimmed tree" << std::endl;
	// ROOT::RDF::RSnapshotOptions options;
	// options.fMode = "UPDATE";
	// df_neutrons.Snapshot("response",outputfile,{"tdiff","tof","beta","gamma","distance","light","psd","nerg_classical","nerg_relativistic"},options);
}
