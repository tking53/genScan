#include <TNtupleD.h>
#include <cmath>
#include <iomanip>
#include <limits>
#include <mutex>
#include <random>
#include <set>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/describe.hpp>

#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/parse.h>

#include "StringManipFunctions.hpp"
#include "TROOT.h"
#include "ROOT/TBufferMerger.hxx"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TNtuple.h"

struct RootHisSettings{
	std::string inputfile;
	std::string axis;
	std::string hisname;
	std::vector<std::string> gate;
	std::vector<int> projection_indices;
	std::pair<double,double> gate_bounds;
	double low;
	double high;
	int xrebin;
	int yrebin;
};

//need to change this to be shifting steps not drastic changes
template<class T>
class LimitedValue {
	public:
		LimitedValue() {
		}

		LimitedValue(const T& v,const std::pair<T,T>& l) : 
			value(v), 
			next_value(v), 
			limits(l), 
			delta(0.0,0.0001) 
		{
		}	

		LimitedValue(const LimitedValue& rhs) : 
			value(rhs.value), 
			next_value(rhs.next_value), 
			limits(rhs.limits), 
			delta(rhs.delta)
		{
		}
		LimitedValue(LimitedValue&& rhs) noexcept : 
			value(rhs.value), 
			next_value(rhs.next_value), 
			limits(std::move(rhs.limits)),
			delta(std::move(rhs.delta))
		{
		}	
		LimitedValue& operator=(const LimitedValue& other){
			if( this != &other ){
				this->value = other.value;
				this->next_value = other.next_value;
				this->limits = other.limits;
				this->delta = other.delta;
			}
			return *this;
		}
		LimitedValue& operator=(LimitedValue&& other) noexcept{
			if( this != &other ){
				this->value = other.value;
				this->next_value = other.next_value;
				this->limits = std::move(other.limits);
				this->delta = std::move(other.delta);
			}
			return *this;
		}

		const T& GetValue() const{
			return this->value;
		}

		const std::pair<T,T> GetLimits() const{
			return this->limits;
		}

		bool IsWithinLimits(const T& test) const{
			return test >= this->limits.first and test <= this->limits.second;
		}

		bool IsValueWithinLimits() const {
			return value >= this->limits.first and value <= this->limits.second;
		}

		void SwapValues() {
			std::swap(this->value,this->next_value);
		}

		void ProposeNextValue(std::mt19937_64& gen){
			this->next_value = this->value + delta(gen);
		}

	private:
		T value;
		T next_value;
		std::pair<T,T> limits;
		std::normal_distribution<T> delta;
};

template<class T>
class BkgTerm {
	public:
		BkgTerm(const std::vector<LimitedValue<T>>& p) : pars(p){
		}
		virtual ~BkgTerm() = default;
		virtual T Evaluate(const T& x) const{
			return 0.0;
		}
		virtual void ProposeNewPars(std::mt19937_64& gen) final{
			for( auto& p : this->pars ){
				p.ProposeNextValue(gen);
			}
		}	
		virtual void SwapValues() final{
			for( auto& p : this->pars ){
				p.SwapValues();
			}
		}
	protected:
		std::vector<LimitedValue<T>> pars;
};

template<class T>
class FlatBkg : public BkgTerm<T>{
	public:
		FlatBkg(const std::vector<LimitedValue<T>>& p) : BkgTerm<T>(p) {
		}
		virtual ~FlatBkg() = default;
		virtual T Evaluate(const T& x) const{
			return this->pars[0].GetValue();
		}
};

template<class T>
class SlopeBkg : public BkgTerm<T>{
	public:
		SlopeBkg(const std::vector<LimitedValue<T>>& p) : BkgTerm<T>(p) {
		}
		virtual ~SlopeBkg() = default;
		virtual T Evaluate(const T& x) const{
			return this->pars[0].GetValue()-this->pars[1].GetValue()*std::abs(x);
		}
};

template<class T>
class ExpCuspBkg : public BkgTerm<T> {
	public:
		ExpCuspBkg(const std::vector<LimitedValue<T>>& p) : BkgTerm<T>(p) {
		}
		virtual ~ExpCuspBkg() = default;
		virtual T Evaluate(const T& x) const{
			return this->pars[0].GetValue()+std::exp(this->pars[1].GetValue()-this->pars[2].GetValue()*std::abs(x));
		}
};

template<class T>
class Isotope {
	public:
		Isotope(const std::string& n,const LimitedValue<T>& hl,const LimitedValue<T>& e,const LimitedValue<T>& b) : 
			name(n), 
			half_life(hl), 
			efficiency(e),
			branching_ratio(b)
		{

		}

		Isotope(const Isotope<T>& rhs) : 
			name(rhs.name),
			half_life(rhs.half_life),
			efficiency(rhs.efficiency),
			branching_ratio(rhs.branching_ratio),
			daughters(rhs.daughters)
		{
		}

		Isotope(Isotope<T>&& rhs) noexcept : 
			name(std::move(rhs.name)),
			half_life(std::move(rhs.half_life)),
			efficiency(std::move(rhs.efficiency)),
			branching_ratio(std::move(rhs.branching_ratio)),
			daughters(std::move(rhs.daughters))
		{
		}

		Isotope<T>& operator=(const Isotope<T>& other){
			if( this != &other){
				this->name = other.name;
				this->half_life = other.half_life;
				this->efficiency = other.efficiency;
				this->daughters = other.daughters;
				this->branching_ratio = other.branching_ratio;
			}
			return *this;
		}

		Isotope<T>& operator=(Isotope<T>&& other){
			if( this != &other){
				this->name = std::move(other.name);
				this->half_life = std::move(other.half_life);
				this->efficiency = std::move(other.efficiency);
				this->daughters = std::move(other.daughters);
				this->branching_ratio = std::move(other.branching_ratio);
			}
			return *this;
		}

		void AddDaughter(const std::string& n,
				const LimitedValue<T>& hl,
				const LimitedValue<T>& e,
				const LimitedValue<T>& br){
			this->daughters.push_back(new Isotope(n,hl,e,br));
		}

		Isotope<T>* GetDaughter(size_t idx){
			return this->daughters[idx];
		}

		const LimitedValue<T>& GetBranchingRatio(size_t idx) const{
			return this->branching_ratio[idx];
		}

		size_t GetNumDaughters() const{
			return this->daughters.size();
		}

		const std::string& GetName() const{
			return this->name;
		}

		void display_names(){
			this->display_helper(this,0,"",
					[](const Isotope* p){ return p->GetName();}
					);
		}

		void display_half_lives(){
			this->display_helper(this,0,"",
					[](const Isotope* p){ return p->GetHalfLife();}
					);
		}

		void display_efficiencies(){
			this->display_helper(this,0,"",
					[](const Isotope* p){ return p->GetEfficiency();}
					);
		}

		void display_branching_ratios(){
			this->display_helper(this,0,"",
					[](const Isotope* p){ return p->GetBranchingRatio();}
					);
		}

		const T& GetHalfLife() const{
			return this->half_life.GetValue();
		}

		bool IsHalfLifeWithinLimits() const{
			return this->half_life.IsValueWithinLimits();
		}

		const T GetLambda() const{
			return std::log(2.0)/this->half_life.GetValue();
		}

		const T& GetEfficiency() const{
			return this->efficiency.GetValue();
		}

		bool IsEfficiencyWithinLimits() const{
			return this->efficiency.IsValueWithinLimits();
		}

		const T& GetBranchingRatio() const{
			return this->branching_ratio.GetValue();
		}

		bool IsBranchingRatioWithinLimits() const{
			return this->branching_ratio.IsValueWithinLimits();
		}

		bool IsIsotopeWithinLimits() const{
			// if( not this->IsBranchingRatioWithinLimits() ){
			// 	spdlog::critical("{} fails on BR {}",this->name,this->GetBranchingRatio());
			// }
			// if( not this->IsHalfLifeWithinLimits() ){
			// 	spdlog::critical("{} fails on HL {}",this->name,this->GetHalfLife());
			// }
			// if( not this->IsEfficiencyWithinLimits() ){
			// 	spdlog::critical("{} fails on EF {}",this->name,this->GetEfficiency());
			// }
			return this->IsBranchingRatioWithinLimits() and 
			       this->IsHalfLifeWithinLimits() and 
			       this->IsEfficiencyWithinLimits();
		}

		T Exp(const T& x) const{
			if( x >= 0.0 ){
				return std::exp(-x*std::log(2.0)/this->half_life.GetValue());
			}else{
				return 0.0;
			}
		}

		void ProposeNewHalfLife(std::mt19937_64& gen){
			this->half_life.ProposeNextValue(gen);
		}

		void ProposeNewEfficiency(std::mt19937_64& gen) {
			this->efficiency.ProposeNextValue(gen);
		}

		void ProposeNewBranchingRatio(std::mt19937_64& gen){
			this->branching_ratio.ProposeNextValue(gen);
		}

		void SwapValues() {
			this->half_life.SwapValues();
			this->branching_ratio.SwapValues();
			this->efficiency.SwapValues();
		}

	private:
		template<class U>
		void display_helper(Isotope* parent,int column,std::string p,U const& f){
			std::string prefix = p;
			if( column > 0 ){
				prefix += std::string(parent->name.length(),' ')+"|";
			}
			if( column > 0 ){
				std::cout << prefix << "--->" << f(this) << "\n";
			}else{
				std::cout << prefix << f(this) << "\n";
			}
			for( const auto& d : this->daughters ){
				d->display_helper(d,column+1,prefix,f);
			}
		}

		std::string name;
		LimitedValue<T> half_life;
		LimitedValue<T> efficiency;
		LimitedValue<T> branching_ratio;

		std::vector<Isotope*> daughters;
};

template<class T>
class DecayCurve{
	public:
		DecayCurve() = default;
		DecayCurve(const std::vector<T>& x,const std::vector<T>& y) : xvals(x), yvals(y) {
		}
		DecayCurve(std::vector<T>&& x,std::vector<T>&& y) : xvals(std::move(x)), yvals(std::move(y)) {
		}
		DecayCurve(const std::vector<T>& x,const BkgTerm<T>& func) : xvals(x) {
			for( const auto& xx : this->xvals ) {
				this->yvals.push_back(func.Evaluate(xx));
			}
		}
		~DecayCurve() = default;
		DecayCurve(const DecayCurve&) = default;
		DecayCurve(DecayCurve&&) noexcept = default;
		DecayCurve& operator=(const DecayCurve&) = default;
		DecayCurve& operator=(DecayCurve&&) noexcept = default;
	
		DecayCurve operator+(const DecayCurve& rhs) const{
			DecayCurve tmp = *this;
			tmp += rhs;
			return tmp;
		}

		DecayCurve& operator+=(const DecayCurve& rhs) {
			for( size_t ii = 0; ii < this->yvals.size(); ++ii ){
				this->yvals[ii] += rhs.yvals[ii];
			}
			return *this;
		}

		DecayCurve operator-(const DecayCurve& rhs) const{
			DecayCurve tmp = *this;
			tmp -= rhs;
			return tmp;
		}

		DecayCurve& operator-=(const DecayCurve& rhs) {
			for( size_t ii = 0; ii < this->yvals.size(); ++ii ){
				this->yvals[ii] -= rhs.yvals[ii];
			}
			return *this;
		}

		DecayCurve operator*(const DecayCurve& rhs) const{
			DecayCurve tmp = *this;
			tmp *= rhs;
			return tmp;
		}

		DecayCurve& operator*=(const DecayCurve& rhs) {
			for( size_t ii = 0; ii < this->yvals.size(); ++ii ){
				this->yvals[ii] *= rhs.yvals[ii];
			}
			return *this;
		}

		DecayCurve operator/(const DecayCurve& rhs) const{
			DecayCurve tmp = *this;
			tmp /= rhs;
			return tmp;
		}

		DecayCurve& operator/=(const DecayCurve& rhs) {
			for( size_t ii = 0; ii < this->yvals.size(); ++ii ){
				this->yvals[ii] /= rhs.yvals[ii];
			}
			return *this;
		}

		const std::vector<T>& GetXVals() const{
			return this->xvals;
		}

		const std::vector<T>& GetYVals() const{
			return this->yvals;
		}

		size_t GetNumVals() const {
			return this->xvals.size();
		}

		std::pair<T,T> GetPoint(size_t idx) const{
			return std::pair<T,T>(this->xvals[idx],this->yvals[idx]);
		}

	private:
		std::vector<T> xvals;
		std::vector<T> yvals;
};

template<class T>
LimitedValue<T> generate_limited_value_from_yaml(const YAML::Node& node){
	if( auto constrain = node["Constrain"] ){
		if( auto range = constrain["Range"] ){
			auto min_val = range["Min"].as<T>();
			auto max_val = range["Max"].as<T>();
			return LimitedValue<T>((min_val+max_val)/2.0,{min_val,max_val});
		}else{
			spdlog::error("unable to create LimitedValue<T> from yaml node missing Range:");
			throw std::runtime_error("unable to make LimitedValue<T> from yaml node missing Range:");
		}
	}else{
		spdlog::error("unable to create LimitedValue<T> from yaml node missing Constrain:");
		throw std::runtime_error("unable to make LimitedValue<T> from yaml node missing Constrain:");
	}	
}

class DecayNetwork{
	public:
		DecayNetwork(const std::string& inputfile,const RootHisSettings& root_info){
			YAML::Node doc = YAML::LoadFile(inputfile);
			if( auto bkg = doc["Background"] ){
				this->LoadBkgFromYaml(bkg,inputfile);
			}else{
				spdlog::error("No Background tag in config file: {}",inputfile);
				throw std::runtime_error("No Background tag in yaml");
			}
			if( auto number = doc["Number"] ){
				this->LoadNumberFromYaml(number,inputfile);
			}else{
				spdlog::error("No Number tag in config file: {}",inputfile);
				throw std::runtime_error("No Number tag in yaml");
			}
			if( auto isotope = doc["Isotope"] ){
				this->LoadIsotopeFromYaml(isotope,inputfile);
			}else{
				spdlog::error("No Isotope tag in config file: {}",inputfile);
				throw std::runtime_error("No Isotope tag in yaml");
			}
			this->LoadRootSettings(root_info);
			
			this->workspace = std::vector<double>(this->xvals.size(),0.0);
			this->components["Bkg"] = DecayCurve<double>(this->xvals,*(this->bkg));
			this->keys.insert("Bkg");
			this->GenerateKeys();
			this->Evaluate();
			this->keys.insert("Total");
		}

		DecayNetwork(const LimitedValue<double>& n,const std::vector<double>& x) : number(n), xvals(x){
			this->parent = new Isotope<double>("Cu78",
					LimitedValue<double>(0.3313,{0.3,0.36}),
					LimitedValue<double>(0.6,{0.2,1.0}),
					LimitedValue<double>(1.0,{1.0,1.0}));

			this->parent->AddDaughter("Zn78",
					LimitedValue<double>(1.47,{1.3,1.6}),
					LimitedValue<double>(0.6,{0.2,1.0}),
					LimitedValue<double>(0.5,{0.0,1.0}));
			this->parent->AddDaughter("Zn77",
					LimitedValue<double>(20.09,{2.0,2.1}),
					LimitedValue<double>(0.6,{0.2,1.0}),
					LimitedValue<double>(0.5,{0.0,1.0}));
			parent->GetDaughter(0)->AddDaughter("Ga78",
					LimitedValue<double>(5.09,{5.0,5.2}),
					LimitedValue<double>(0.6,{0.2,1.0}),
					LimitedValue<double>(0.5,{0.0,0.1}));

			this->bkg = new SlopeBkg<double>({{10.0,{0.0,100.0}},{0.0,{0.0,0.0}}});
			this->components["Bkg"] = DecayCurve<double>(this->xvals,*(this->bkg));
			this->keys.insert("Bkg");

			this->GenerateKeys();

			this->Evaluate();
			this->keys.insert("Total");
		}

		const DecayCurve<double>& GetCurve(const std::string& name) const {
			return this->components.at(name);
		}

		const std::set<std::string>& GetKeys() const{
			return this->keys;
		}
		
		const std::vector<std::string>& GetIsotopes() const{
			return this->isotopes;
		}

		void DisplayKeys() const{
			for( const auto& k : this->keys ){
				std::cout << k << std::endl;
			}
		}

		void DisplayNames() const{
			this->parent->display_names();
		}

		void DisplayHalfLives() const{
			this->parent->display_half_lives();
		}

		void DisplayBranchingRatios() const{
			this->parent->display_branching_ratios();
		}

		void DisplayEfficiencies() const{
			this->parent->display_efficiencies();
		}

		double log_posterior() const{
			auto lp = this->log_prior();
			// spdlog::info("lp : {}",lp);
			if( not std::isfinite(lp) ){
				return -std::numeric_limits<double>::infinity();
			}

			//return lp + this->log_likelihood();
			return lp + this->log_likelihood_poisson();
		}

		//sigma is the sticking point
		double log_likelihood() const{
			double ll = 0.0;
			auto total = this->GetCurve("Total");
			auto diff = this->data - total;
			auto y = diff.GetYVals();
			for( const auto& r : y ){
				ll -= 0.5*r*r/this->sigma;
			}
			ll -= y.size()*std::log(this->sigma);
			return ll;
		}

		double log_likelihood_poisson() const{
			double ll = 0.0;
			auto total = this->GetCurve("Total");
			for( size_t ii = 0; ii < this->xvals.size(); ++ii ){
				auto mu = total.GetPoint(ii).second;
				// spdlog::info("ii:{} k:{} mu:{}",ii,this->yvals[ii],mu);
				ll += this->yvals[ii]*std::log(mu) - mu;
			}
			return ll;
		}

		double log_prior() const{
			if( this->number.GetValue() <= 0.0 ){
				// spdlog::critical("number is bad {}",this->number.GetValue());
				return -std::numeric_limits<double>::infinity();
			}
			double retval = 0.0;
			this->log_prior(this->parent,retval);
			return retval;
		}

		void propose(std::mt19937_64& gen) {
			this->number.ProposeNextValue(gen);
			this->number.SwapValues();

			this->parent->ProposeNewEfficiency(gen);
			this->parent->ProposeNewHalfLife(gen);
			this->parent->SwapValues();

			this->bkg->ProposeNewPars(gen);
			this->bkg->SwapValues();

			for( size_t ii = 0; ii < this->parent->GetNumDaughters(); ++ii ){
				this->propose(gen,this->parent->GetDaughter(ii));
			}
		}

		void undo_proposition() {
			this->number.SwapValues();
			this->bkg->SwapValues();
			this->undo_proposition(this->parent);
		}

		void Evaluate() {
			//this only get's evaluated when we do a new proposition
			this->components["Bkg"] = DecayCurve<double>(this->xvals,*(this->bkg));
		 	const double A0 = this->number.GetValue()*this->parent->GetLambda();
			for( size_t ii = 0; ii < this->xvals.size(); ++ii ){
				this->workspace[ii] = (A0*
						this->parent->GetEfficiency()*
						this->parent->GetBranchingRatio()*
						this->parent->Exp(this->xvals[ii])
					   );
			}
			this->components[this->parent->GetName()] = DecayCurve<double>(this->xvals,this->workspace);

			for( size_t ii = 0; ii < this->parent->GetNumDaughters(); ++ii ){
				std::vector<Isotope<double>*> chain = {this->parent};
				this->Evaluate(this->parent->GetDaughter(ii),chain);
			}	

			for( size_t ii = 0; ii < this->xvals.size(); ++ii ){
				this->workspace[ii] = 0.0;
			}
			for( const auto& kv : this->isotopes ){
				auto curve = this->GetCurve(kv);
				for( size_t ii = 0; ii < this->xvals.size(); ++ii ){
					const auto& [x,y] = curve.GetPoint(ii);
					this->workspace[ii] += y;
				}
			}
			auto curve = this->GetCurve("Bkg");
			for( size_t ii = 0; ii < this->xvals.size(); ++ii ){
				const auto& [x,y] = curve.GetPoint(ii);
				this->workspace[ii] += y;
			}

			this->components["Total"] = DecayCurve<double>(this->xvals,this->workspace);
		}

		void GenerateKeys() {
			this->isotopes.push_back(this->parent->GetName());
			this->keys.insert(this->parent->GetName());
			this->values.push_back({this->parent->GetName(),this->parent});
			for( size_t ii = 0; ii < this->parent->GetNumDaughters(); ++ii ){
				this->GenerateKeys(this->parent->GetDaughter(ii));
			}
		}

		size_t GetNumVals() const {
			return this->xvals.size();
		}

		double GetData(size_t idx) const{
			return this->yvals[idx];
		}

		double GetNumber() const{
			return this->number.GetValue();
		}

		double GetHalfLife(const std::string& key) const{
			for( const auto& kv : this->values ){
				if( kv.first.compare(key) == 0 ){
					return kv.second->GetHalfLife();
				}
			}
			return -std::numeric_limits<double>::infinity();
		}

		double GetEfficiency(const std::string& key) const{
			for( const auto& kv : this->values ){
				if( kv.first.compare(key) == 0 ){
					return kv.second->GetEfficiency();
				}
			}
			return -std::numeric_limits<double>::infinity();
		}

		double GetBranchingRatio(const std::string& key) const{
			for( const auto& kv : this->values ){
				if( kv.first.compare(key) == 0 ){
					return kv.second->GetBranchingRatio();
				}
			}
			return -std::numeric_limits<double>::infinity();
		}

	private:
		LimitedValue<double> number;
		std::vector<double> xvals;
		std::vector<double> yvals;
		std::vector<double> yerrs;
		std::vector<double> workspace;

		DecayCurve<double> data;
		double sigma;

		std::set<std::string> keys;
		std::vector<std::string> isotopes;

		Isotope<double>* parent;
		BkgTerm<double>* bkg;
		std::map<std::string,DecayCurve<double>> components;
		std::vector<std::pair<std::string,Isotope<double>*>> values;

		void LoadRootSettings(const RootHisSettings& root_info){
			auto rfile = new TFile(root_info.inputfile.c_str(),"READ");
			auto mainhis = rfile->Get(root_info.hisname.c_str());
			TH1* histofit = nullptr;;
			if( mainhis != nullptr ){
				auto histype = std::string(mainhis->ClassName());
				boost::regex re2d("TH2");
				boost::regex re1d("TH1");
				if( boost::regex_search(histype, re2d) ){
					if( root_info.projection_indices.size() == 2 ){
						auto name = std::string(mainhis->GetName())+"_proj_"+root_info.axis;
						if( root_info.axis.compare("x") == 0 ){
							histofit = dynamic_cast<TH2*>(mainhis)->ProjectionX(name.c_str(),root_info.projection_indices[0],root_info.projection_indices[1]);
							if( root_info.xrebin > 0 ){
								histofit->RebinX(root_info.xrebin);
							}
						}else{
							histofit = dynamic_cast<TH2*>(mainhis)->ProjectionY(name.c_str(),root_info.projection_indices[0],root_info.projection_indices[1]);
							if( root_info.yrebin > 0 ){
								histofit->RebinX(root_info.yrebin);
							}
						}	
					}
					if( root_info.gate.size() == 2 ){
						auto name = std::string(mainhis->GetName())+"_gate_"+root_info.axis;
						auto g = root_info.gate_bounds;
						if( root_info.axis.compare("x") == 0 ){
							auto minbin = dynamic_cast<TH2*>(mainhis)->GetYaxis()->FindBin(g.first);
							auto maxbin = dynamic_cast<TH2*>(mainhis)->GetYaxis()->FindBin(g.second);
							histofit = dynamic_cast<TH2*>(mainhis)->ProjectionY(name.c_str(),minbin,maxbin);
							if( root_info.xrebin > 0 ){
								histofit->RebinX(root_info.xrebin);
							}
						}else{
							auto minbin = dynamic_cast<TH2*>(mainhis)->GetXaxis()->FindBin(g.first);
							auto maxbin = dynamic_cast<TH2*>(mainhis)->GetXaxis()->FindBin(g.second);
							histofit = dynamic_cast<TH2*>(mainhis)->ProjectionX(name.c_str(),minbin,maxbin);
							if( root_info.yrebin > 0 ){
								histofit->RebinX(root_info.yrebin);
							}
						}
					}
				}else if( boost::regex_search(histype,re1d) ){
					histofit = dynamic_cast<TH1*>(mainhis);
				}else{
					throw std::runtime_error("not passed a TH1 or TH2 histogram");
				}
			}else{
				spdlog::error("histogram {} does not exist in root file {}",root_info.hisname,root_info.inputfile);
				throw std::runtime_error("his does not exist");
			}
			if( histofit != nullptr ){
				auto lowbin = histofit->FindBin(root_info.low);
				auto highbin = histofit->FindBin(root_info.high);
				for( int ii = lowbin; ii < highbin; ++ii ){
					this->xvals.push_back(histofit->GetBinCenter(ii));
					this->yvals.push_back(histofit->GetBinContent(ii));
					this->yerrs.push_back(histofit->GetBinError(ii));
				}
				this->data = DecayCurve<double>(this->xvals,this->yvals);
			}else{
				spdlog::error("retreived histogram {} exists, but failed to retrieve from root file {}",
						root_info.hisname,root_info.inputfile);
				throw std::runtime_error("issue loading his");
			}
		}
		
		void LoadIsotopeFromYaml(const YAML::Node& isotope,const std::string& inputfile){
			auto name = isotope["Name"].as<std::string>();
			auto hl = generate_limited_value_from_yaml<double>(isotope["HalfLife"]);
			auto eff = generate_limited_value_from_yaml<double>(isotope["Efficiency"]);
			auto br = generate_limited_value_from_yaml<double>(isotope["BranchingRatio"]); 
			this->parent = new Isotope<double>(name,hl,eff,br);
			if( auto child = isotope["Isotope"] ){
				for( size_t ii = 0; ii < child.size(); ++ii ){
					this->LoadIsotopeFromYaml(child[ii],inputfile,this->parent);
				}
			}
		}

		void LoadIsotopeFromYaml(const YAML::Node& isotope,const std::string& inputfile,Isotope<double>* p){
			auto name = isotope["Name"].as<std::string>();
			auto hl = generate_limited_value_from_yaml<double>(isotope["HalfLife"]);
			auto eff = generate_limited_value_from_yaml<double>(isotope["Efficiency"]);
			auto br = generate_limited_value_from_yaml<double>(isotope["BranchingRatio"]); 
			p->AddDaughter(name,hl,eff,br);
			if( auto child = isotope["Isotope"] ){
				for( size_t ii = 0; ii < child.size(); ++ii ){
					this->LoadIsotopeFromYaml(child[ii],inputfile,p->GetDaughter(p->GetNumDaughters()-1));
				}
			}
		}
		
		void LoadNumberFromYaml(const YAML::Node& number,const std::string& inputfile){
			this->number = generate_limited_value_from_yaml<double>(number);
		}

		void LoadBkgFromYaml(const YAML::Node& bkg,const std::string& inputfile){
			auto pars = bkg["Parameter"];
			if( not pars ){
				spdlog::error("Background Model missing Parameter list in config: {}",inputfile);
				throw std::runtime_error("Background Model missing Parameter list in config");
			}
			auto bkg_model = StringManip::tolower(bkg["Model"].as<std::string>("unknown"));
			std::map<std::string,LimitedValue<double>> pmap;
			for( size_t ii = 0; ii < pars.size(); ++ii ){
				auto name = StringManip::tolower(pars[ii]["Name"].as<std::string>("unknown"));
				pmap[name] = generate_limited_value_from_yaml<double>(pars[ii]);
			}	
			if( bkg_model.compare("constant") == 0 ){
				if( pmap.find("constant") == pmap.end() ){
					spdlog::error("constant background model, expected Name: constant field");
					throw std::runtime_error("constant background model missing Name: constant field");
				}else{
					this->bkg = new FlatBkg<double>({pmap["constant"]});
				}
			}else if( bkg_model.compare("linear") == 0 ){
				if( pmap.find("constant") == pmap.end() ){
					spdlog::error("linear background model, expected Name: constant field");
					throw std::runtime_error("linear background model missing Name: constant field");
				}else{
					if( pmap.find("slope") == pmap.end() ){
						spdlog::error("linear background model, expected Name: slope field");
						throw std::runtime_error("linear background model missing Name: slope field");
					}else{
						this->bkg = new SlopeBkg<double>({pmap["constant"],pmap["slope"]});
					}
				}
			}else if( bkg_model.compare("exp_cusp") == 0 ){
				if( pmap.find("offset") == pmap.end() ){
					spdlog::error("exp_cusp background model, expected Name: offset field");
					throw std::runtime_error("exp_cusp background model missing Name: offset field");
				}else{
					if( pmap.find("constant") == pmap.end() ){
						spdlog::error("exp_cusp background model, expected Name: constant field");
						throw std::runtime_error("exp_cusp background model missing Name: constant field");
					}else{
						if( pmap.find("slope") == pmap.end() ){
							spdlog::error("exp_cusp background model, expected Name: slope field");
							throw std::runtime_error("exp_cusp background model missing Name: slope field");
						}else{
							this->bkg = new SlopeBkg<double>({pmap["offset"],pmap["constant"],pmap["slope"]});
						}
					}
				}
			}else{
				spdlog::error("unknown background model, expected either constant, linear, exp_cusp name");
				throw std::runtime_error("unknown background model type");
			}
		}

		void GenerateKeys(Isotope<double>* d){
			this->isotopes.push_back(d->GetName());
			this->keys.insert(d->GetName());
			this->values.push_back({d->GetName(),d});
			for( size_t ii = 0; ii < d->GetNumDaughters(); ++ii ){
				this->GenerateKeys(d->GetDaughter(ii));
			}
		}

		void propose(std::mt19937_64& gen,Isotope<double>* d) {
			d->ProposeNewHalfLife(gen);
			d->ProposeNewEfficiency(gen);
			d->ProposeNewBranchingRatio(gen);
			d->SwapValues();
			for( size_t ii = 0; ii < d->GetNumDaughters(); ++ii ){
				this->propose(gen,d->GetDaughter(ii));
			}
		}

		void log_prior(Isotope<double>* d,double& retval) const{
			if( not d->IsIsotopeWithinLimits() ){
				retval = -std::numeric_limits<double>::infinity();
				return;
			}else{
				for( size_t ii = 0; ii < d->GetNumDaughters(); ++ii ){
					this->log_prior(d->GetDaughter(ii),retval);
				}
			}
		}

		void undo_proposition(Isotope<double>* d){
			d->SwapValues();
			for( size_t ii = 0; ii < d->GetNumDaughters(); ++ii ){
				this->undo_proposition(d->GetDaughter(ii));
			}
		}

		void Evaluate(Isotope<double>* d,std::vector<Isotope<double>*> chain){
			chain.push_back(d);

			std::vector<double> lambdas;
			for( const auto& c : chain ){
				lambdas.push_back(c->GetLambda());
			}
			double l_prod = this->number.GetValue()*d->GetBranchingRatio()*d->GetEfficiency();
			if( l_prod > 0.0 ){
				for( const auto& l : lambdas ){
					l_prod *= l;
				}

				for( size_t kk = 0; kk < this->xvals.size(); ++kk ){
					auto sum = 0.0;
					for( size_t ii = 0; ii < chain.size(); ++ii ){
						auto denom = 1.0;
						for( size_t jj = 0; jj < chain.size(); ++jj ){
							if( ii != jj ){
								auto diff = (lambdas[jj] - lambdas[ii]);
								//this is a sticking point
								//we need to actually generate the 
								//transmuation matrix to avoid this issue
								if( abs(diff) > 0.0 ){
									denom *= (lambdas[jj] - lambdas[ii]);
								}else{
									denom *= 1.0e-16;
								}
							}
						}
						sum += chain[ii]->Exp(this->xvals[kk])/denom;
					}	
					sum *= l_prod;
					this->workspace[kk] = sum;
				}
			}else{
				for( size_t kk = 0; kk < this->xvals.size(); ++kk ){
					this->workspace[kk] = 0.0;
				}
			}
			this->components[d->GetName()] = DecayCurve<double>(this->xvals,this->workspace);

			for( size_t ii = 0; ii < d->GetNumDaughters(); ++ii ){
				this->Evaluate(d->GetDaughter(ii),chain);
			}
		}
};

int main(int argc, char *argv[]) {
	std::string outputprefix;
	std::string configfile;
	RootHisSettings root_info;
	bool quiet;
	bool chi2;
	bool storechi2;
	size_t nthreads;
	size_t ntrials;
	size_t thin;
	size_t burnin;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("axis,a",boost::program_options::value<std::string>(&root_info.axis)->default_value("x"),"axis to project onto (x,y,X,Y) if 2D")
		("burnin,b",boost::program_options::value<size_t>(&burnin)->default_value(1000),"number to burnin the random number generation (done by each thread)")
		("chi2,c",boost::program_options::value<bool>(&chi2)->default_value(true),"chi2 fit, or loglikelihood")
		("data,d",boost::program_options::value<std::string>(&root_info.hisname),"histogram to manipulate")
		("configfile,f",boost::program_options::value<std::string>(&configfile),"yaml file to read the decay configuration and fit settings from")
		("gate,g",boost::program_options::value<std::vector<std::string>>(&root_info.gate)->multitoken(),"values to gate within in 2d histogram")
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&root_info.inputfile),"file to get the histogram from")
		("lowerbound,l",boost::program_options::value<double>(&root_info.low),"lower bound to perform fit")
		("thinning,m",boost::program_options::value<size_t>(&thin)->default_value(1000),"modulo used to determine if a trial should be recorded")
		("ntrials,n",boost::program_options::value<size_t>(&ntrials)->default_value(10000),"number of trials to perform when fitting")
		("outputprefix,o",boost::program_options::value<std::string>(&outputprefix)->default_value("GenMCHalfLife"),"file to output to fit info to")
		("projectionindex,p",boost::program_options::value<std::vector<int>>(&root_info.projection_indices)->multitoken(),"index limits to project on if 2d histogram")
		("quiet,q",boost::program_options::value<bool>(&quiet)->default_value(false),"quiet output")
		("storechi2,s",boost::program_options::value<bool>(&storechi2)->default_value(true),"store chi2 plot")
		("nthreads,t",boost::program_options::value<size_t>(&nthreads)->default_value(std::thread::hardware_concurrency()/2),"number of threads used in parallel")
		("upperbound,u",boost::program_options::value<double>(&root_info.high),"upper bound to perform fit")
		("xrebin,x",boost::program_options::value<int>(&root_info.xrebin)->default_value(0),"rebin factor for the x direction")
		("yrebin,y",boost::program_options::value<int>(&root_info.yrebin)->default_value(0),"rebin factor for the y direction")
		;


	boost::program_options::positional_options_description p;

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

		auto numproj = root_info.projection_indices.size();

		if( not vm.count("lowerbound") ){
			spdlog::error("missing lowerbound");
			exit(EXIT_FAILURE);
		}
	        if( not	vm.count("upperbound") ){
			spdlog::error("missing upperbound");
			exit(EXIT_FAILURE);
		}	

		if( not vm.count("data") ){
			spdlog::error("Not provided histogram to fit");
			exit(EXIT_FAILURE);
		}

		if( not vm.count("inputfile") ){
			spdlog::error("Not provided inputfile containing histogram to fit");
			exit(EXIT_FAILURE);
		}

		if( not vm.count("configfile") ){
			spdlog::error("No config yaml file provided");
			exit(EXIT_FAILURE);
		}
	
		//double checking thread limitations
		auto thread_limit = std::thread::hardware_concurrency();
		if( nthreads >= thread_limit ){
			spdlog::error("thread count: {} exceeds system limit of {} threads",nthreads,thread_limit);
			exit(EXIT_FAILURE);
		}
		if( nthreads < 1 ){
			spdlog::error("thread count: {} < 1 ",nthreads);
			exit(EXIT_FAILURE);
		}

		// axis = StringManip::tolower(axis);
		// if( axis.compare("x") != 0 and axis.compare("y") != 0 ){
		// 	spdlog::error("unknown axis projection : {}",axis);
		// 	exit(EXIT_FAILURE);
		// }

		// gatevalues = ParseGates(gates);
	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    


	//need to strip the histogram into something we can use for fitting
	//we should probably not allow this program to do multiple fits at once
	//because of the sheer nature of monte carlo, but we need to limit ourself to either be 
	//a single projection of a single gate

	// std::vector<double> xvals;
	// double x = -50.0;
	// const double dx = 0.001;
	// while( x < 50.0 ){
	// 	xvals.push_back(x);
	// 	x += dx;
	// }
	
	// DecayNetwork dn(LimitedValue<double>(1.0e3,{0.0,1.0e6}),xvals);

	// auto bkg = dn.GetCurve("Bkg");
	// auto cu78 = dn.GetCurve("Cu78");
	// auto zn78 = dn.GetCurve("Zn78");
	// auto ga78 = dn.GetCurve("Ga78");
	// auto zn77 = dn.GetCurve("Zn77");
	// auto total = dn.GetCurve("Total");
	// auto numpts = bkg.GetNumVals();
	// for( size_t ii = 0; ii < numpts; ++ii ){
	// 	const auto& [x,y] = bkg.GetPoint(ii);
	// 	const auto& [cu78x,cu78y] = cu78.GetPoint(ii);
	// 	const auto& [zn78x,zn78y] = zn78.GetPoint(ii);
	// 	const auto& [ga78x,ga78y] = ga78.GetPoint(ii);
	// 	const auto& [zn77x,zn77y] = zn77.GetPoint(ii);
	// 	const auto& [tx,ty] = total.GetPoint(ii);
	// 	std::cout << x << " " 
	// 		  << y << " " 
	// 		  << cu78y << " " 
	// 		  << zn78y << " " 
	// 		  << zn77y << " " 
	// 		  << ga78y << " " 
	// 		  << ty << std::endl;
	// }


	DecayNetwork dn(configfile,root_info);

	auto print_dn = [](const DecayNetwork& dn,std::ostream& out){
		auto keys = dn.GetKeys(); 
		auto numpts = dn.GetNumVals();
		out << "#";
		for( const auto& k : keys ){
			out << " " << k;
		}
		out << std::endl;

		for( size_t ii = 0; ii < numpts; ++ii ){
			std::vector<double> vals;
			for( const auto& k : keys ){
				auto curve = dn.GetCurve(k);
				const auto& [x,y] = curve.GetPoint(ii);
				vals.push_back(x);
				vals.push_back(y);
			}
			out << vals[0];
			for( size_t jj = 0; jj < keys.size(); ++jj ){
				out << " " << vals[2*jj+1];
			}
			out << " " << dn.GetData(ii);
			out << std::endl;
		}
	};


	//dn.DisplayNames();
	// dn.DisplayKeys();

	// return 0;

	std::mt19937_64 gen(42);

	std::vector<std::mt19937_64> gen_vec;
	std::vector<DecayNetwork> dn_vec;
	std::vector<double> logp_current;
	for( size_t ii = 0; ii < nthreads; ++ii ){
		gen_vec.push_back(std::mt19937_64(gen()));
		dn_vec.push_back(DecayNetwork(dn));
		dn_vec.back().propose(gen_vec[ii]);
		dn_vec.back().Evaluate();
		logp_current.push_back(dn.log_posterior());
	}

	const std::vector<std::string> elements = { "NN", "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca", "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cp", "Uut", "Uuq", "Uup", "Uuh", "Uus", "Uuo"};


	std::map<std::string,size_t> element_map;
	for( size_t ii = 0; ii < elements.size(); ++ii ){
		element_map[elements[ii]] = ii;
	}

	boost::regex splitter = boost::regex("^([A-Za-z]+)(\\d+)$");

	auto convert_name = [=,&element_map](const std::string& name){
		boost::smatch m;
		std::pair<int,int> AAA_ZZZ = {0,0};
		if( boost::regex_match(name, m, splitter) ){
			std::string element = m[1];
			std::string mass = m[2];
			auto ZZZ = element_map[element];
			auto AAA = std::stoi(mass);
			AAA_ZZZ = {AAA,ZZZ};
		}
		return AAA_ZZZ;
	};

	std::uniform_real_distribution<double> U(0.0, 1.0);

	//this is critical otherwise things don't work
	ROOT::EnableThreadSafety();
	ROOT::TBufferMerger merger("dump.root");

	std::vector<std::thread> workers;
	std::chrono::time_point<std::chrono::high_resolution_clock> global_start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0; ii < nthreads; ++ii ){
		workers.emplace_back(
				[=,&dn_vec,&gen_vec,&logp_current,&U,&print_dn,&convert_name,&merger](){
					auto f = merger.GetFile();
					TNtuple mytuple("simulation","ntuple from mcmchalflifefitter","threadid:idx:aaa:zzz:number:half_life:efficiency:branching_ratio");
					double* vars = new double[8];
					for( size_t jj = 0; jj < (ntrials+burnin); ++jj ){
						dn_vec[ii].propose(gen_vec[ii]);
						dn_vec[ii].Evaluate();
						auto logp_trial = dn_vec[ii].log_posterior();
						double diff = logp_trial - logp_current[ii];
						auto accept_prob = std::exp(diff);
						auto logu = std::log(U(gen_vec[ii]));
						// if( ii == 0 ){
						// std::cout << jj << logp_trial << " " << logp_current[ii] << " " << diff << " " << logu <<  " " << accept_prob << std::endl;
						// }
						//if( U(gen_vec[ii]) < accept_prob ){
						if( logu < diff ){
							// if( ii == 0 ){
							// std::cout << jj << " " << std::setprecision(16) << logp_trial << " " << dn_vec[ii].GetNumber() << " " << dn_vec[ii].GetHalfLife("Cu78") << " " << dn_vec[ii].GetEfficiency("Cu78") << std::endl;
							// }
							logp_current[ii] = logp_trial;
						}else{
							dn_vec[ii].undo_proposition();
						}
						vars[0] = ii;
						vars[1] = jj;
						for( const auto& k : dn_vec[ii].GetIsotopes() ){
							auto aaa_zzz = convert_name(k);
							vars[2] = aaa_zzz.first;
							vars[3] = aaa_zzz.second;
							vars[4] = dn_vec[ii].GetNumber();
							vars[5] = dn_vec[ii].GetHalfLife(k);
							vars[6] = dn_vec[ii].GetEfficiency(k);
							vars[7] = dn_vec[ii].GetBranchingRatio(k);
							mytuple.Fill(vars[0],
								     vars[1],
								     vars[2],
								     vars[3],
								     vars[4],
								     vars[5],
								     vars[6],
								     vars[7]
								     );
						}
						//if( jj%thin == 0 and jj > burnin ){
						//}
					}
					f->Write();
					spdlog::info("thread {} made it to writing",ii);
					// std::ofstream out("thread-"+std::to_string(ii)+".out");
					// print_dn(dn_vec[ii],out);
					// out.close();
					delete [] vars;
				}
				);
	}
	for (auto& t : workers) {
		if (t.joinable()) {
			t.join(); // Wait for the thread to finish
		}
	}

	//for( size_t ii = 0; ii < ntrials; ++ii ){
	//	dn.propose(gen);
	//	dn.Evaluate();
	////	auto logp_trial = dn.log_posterior();
	////	auto accept_prob = std::exp(logp_trial - logp_current);
	////	if( U(gen) < accept_prob ){
	////		logp_current = logp_trial;
	////	}else{
	//	if( U(gen) < 0.9 ){
	//		dn.undo_proposition();
	//	}
	////	}
	//	if( ii%thin == 0 ){
	//		std::cout << ii << std::endl;
	//	}
	////	if( ii > burnin and ii%thin == 0 ){
	////		//record result
	////		dn.RecordValues();
	////	}
	//}
	////dn.CollectStatistics();
	std::chrono::time_point<std::chrono::high_resolution_clock> global_stop_time = std::chrono::high_resolution_clock::now();
	auto global_run_time = global_stop_time - global_start_time;
	const auto hrs = std::chrono::duration_cast<std::chrono::hours>(global_run_time);
	const auto mins = std::chrono::duration_cast<std::chrono::minutes>(global_run_time - hrs);
	const auto secs = std::chrono::duration_cast<std::chrono::seconds>(global_run_time - hrs - mins);
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(global_run_time - hrs - mins - secs);
	// std::cout << "Finished in "
	// 	  << hrs.count() << " hrs "
	// 	  << mins.count() << " mins "
	// 	  << secs.count() << " secs "
	// 	  << ms.count() << " ms" 
	// 	  << std::endl;
}
