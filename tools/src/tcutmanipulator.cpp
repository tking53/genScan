#include <Rtypes.h>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <spdlog/fmt/fmt.h>
#include <sstream>
#include <utility>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <TCutG.h>

#include "CutManager.hpp"

namespace manipulator{
	enum axis{
		X = 0,
		Y = 1
	};

	class operation{
		public:
			operation(axis ax,double xc,double yc) : ax(ax), xcom(xc), ycom(yc){
			}
			virtual void apply(TCutG&) const{
				throw std::runtime_error("Not implemented");
			}
		protected:
			axis ax;
			double xcom;
			double ycom;
	};

	class shift : public operation {
		public:
			shift(axis ax,double xc, double yc, double sv) : operation(ax,xc,yc), value(sv) {
			}
			virtual void apply(TCutG& cut) const{
				auto npts = cut.GetN();
				for( auto ii = 0; ii < npts; ++ii ){
					switch( this->ax ){
						case axis::X:
							cut.SetPointX(ii,cut.GetPointX(ii) + value);
							break;
						case axis::Y:
							cut.SetPointY(ii,cut.GetPointY(ii) + value);
							break;
						default:
							throw std::runtime_error("axis not implemented for shift");
							break;
					}
				}
			}
		protected:
			double value;
	};

	class mirror : public operation {
		public:
			mirror(axis ax,double xc, double yc, double sv) : operation(ax,xc,yc), value(sv) {
			}
			virtual void apply(TCutG& cut) const{
				auto npts = cut.GetN();
				auto svalue = (this->ax == axis::X) ? (value - xcom) : (value - ycom);
				for( auto ii = 0; ii < npts; ++ii ){
					switch( this->ax ){
						case axis::X:
							cut.SetPointX(ii,-1.0*(cut.GetPointX(ii) - xcom)+xcom+2.0*svalue);
							break;
						case axis::Y:
							cut.SetPointY(ii,-1.0*(cut.GetPointY(ii) - ycom)+ycom+2.0*svalue);
							break;
						default:
							throw std::runtime_error("axis not implemented for mirror");
							break;
					}
				}
			}
		protected:
			double value;
	};

	class zero : public operation {
		public:
			zero(axis ax,double xc, double yc) : operation(ax,xc,yc) {
			}
			virtual void apply(TCutG& cut) const{
				auto npts = cut.GetN();
				for( auto ii = 0; ii < npts; ++ii ){
					switch( this->ax ){
						case axis::X:
							cut.SetPointX(ii,cut.GetPointX(ii) - this->xcom);
							break;
						case axis::Y:
							cut.SetPointY(ii,cut.GetPointY(ii) - this->ycom);
							break;
						default:
							throw std::runtime_error("axis not implemented for zero");
							break;
					}
				}
			}
	};

	class mult : public operation {
		public:
			mult(axis ax,double xc,double yc,double value) : operation(ax,xc,yc), value(value) {
			}
			virtual void apply(TCutG& cut) const{
				auto npts = cut.GetN();
				double svalue = (this->ax == axis::X) ? (value*xcom - xcom) : (value*ycom - ycom);
				for( auto ii = 0; ii < npts; ++ii ){
					switch( this->ax ){
						case axis::X:
							cut.SetPointX(ii,cut.GetPointX(ii) + svalue);
							break;
						case axis::Y:
							cut.SetPointY(ii,cut.GetPointY(ii) + svalue);
							break;
						default:
							throw std::runtime_error("axis not implemented for mult");
							break;
					}
				}
			}
		protected:
			double value;
	};
}

void DecodeOperation(const std::string& id,std::vector<manipulator::operation*>& ops,const double& xcom,const double& ycom,const double& xl,const double& xu,const double& yl,const double& yu){
	boost::regex re("([xy])(shift|mirror|zero|mult)(?::([^:\\s]+))?");
	boost::smatch what;
	auto assign_value = [](const std::string& v,const double& xl,const double& xc,const double& xu,const double& yl,const double& yc,const double& yu){
		if( v.compare("xl") == 0 ){
			return xl;
		}else if( v.compare("-xl") == 0 ){
			return -xl;
		}else if( v.compare("xc") == 0 ){
			return xc;
		}else if( v.compare("-xc") == 0 ){
			return -xc;
		}else if( v.compare("xu") == 0 ){
			return xu;
		}else if( v.compare("-xu") == 0 ){
			return -xu;
		}else if( v.compare("yl") == 0 ){
			return yl;
		}else if( v.compare("-yl") == 0 ){
			return -yl;
		}else if( v.compare("yc") == 0 ){
			return yc;
		}else if( v.compare("-yc") == 0 ){
			return -yc;
		}else if( v.compare("yu") == 0 ){
			return yu;
		}else if( v.compare("-yu") == 0 ){
			return -yu;
		}else{
			return std::stod(v);
		}
	};
	if( boost::regex_match(id,what,re) ){
		if( what.size() < 3 or what.size() > 5 ) {
			spdlog::error("invalid operation syntax: {}",id);
			throw std::runtime_error("invalid operation");
		}else{
			auto ax = (what[1].compare("x") == 0) ? manipulator::axis::X : manipulator::axis::Y;
			if( what[2].compare("shift") == 0 ){
				auto val = assign_value(what[3],xl,xcom,xu,yl,ycom,yu);
				ops.push_back(new manipulator::shift(ax,xcom,ycom,val));
			}else if( what[2].compare("mirror") == 0 ){
				auto val = assign_value(what[3],xl,xcom,xu,yl,ycom,yu);
				ops.push_back(new manipulator::mirror(ax,xcom,ycom,val));
			}else if( what[2].compare("mult") == 0 ){
				auto val = assign_value(what[3],xl,xcom,xu,yl,ycom,yu);
				ops.push_back(new manipulator::mult(ax,xcom,ycom,val));
			}else{
				//this is zero
				ops.push_back(new manipulator::zero(ax,xcom,ycom));
			}
		}
	}
}

int main(int argc, char *argv[]) {

	std::string tcutfile;
	std::string outputfile;
	std::vector<std::string> opcodes;
	std::vector<manipulator::operation*> operations;

	std::string operation_message = "operation to perform, they are done in order they are passed to the program";
	operation_message += "\ncurrently supported operations";
	operation_message += "\nxshift:value -> shift the x_com by value";
	operation_message += "\nyshift:value -> shift the y_com by value";
	operation_message += "\nxmirror:value -> reflect the x_com across the x=value line";
	operation_message += "\nymirror:value -> reflect the y_com across the y=value line";
	operation_message += "\nxzero -> shift the x_com to 0.0";
	operation_message += "\nyzero -> shift the y_com to 0.0";
	operation_message += "\nxmult:value -> shift the cut by multiplying x_com by value";
	operation_message += "\nymult:value -> shift the cut by multiplying y_com by value";
	operation_message += "\n value is allowed to be any number, or these special values xl, xc, xu, yl, yc, yu where l,c,u are the lower, center, and upper limits of the respective axis";

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("tcutfile,t",boost::program_options::value<std::string>(&tcutfile),"filename to read tcut from, is expected to be a cxx")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile),"filename to output to, will be a cxx")
		("operation,v",boost::program_options::value<std::vector<std::string>>(&opcodes)->multitoken(),operation_message.c_str())
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

		std::shared_ptr<CUTS::CutRegistry> CutManager(new CUTS::CutRegistry(""));
		CutManager->AddCut("dump",tcutfile);
		auto cut = CutManager->GetCut("dump");
		double xcom;
		double ycom;
		cut->Center(xcom,ycom);
		double xl = xcom;
		double xu = xcom;
		double yl = ycom;
		double yu = ycom;
		auto npts = cut->GetN();
		auto x = xcom;
		auto y = ycom;
		for( auto ii = 0; ii < npts; ++ii ){
			cut->GetPoint(ii,x,y);
			if( x > xu ){
				xu = x;
			}
			if( x < xl ){
				xl = x;
			}
			if( y > yu ){
				yu = y;
			}
			if( y < yl ){
				yl = y;
			}
		}
		for( const auto& id : opcodes ){
			DecodeOperation(id,operations,xcom,ycom,xl,xu,yl,yu);
		}
		for( const auto& op : operations ){
			op->apply(*cut);
		}
		cut->SetLineWidth(4);
		cut->SetLineColor(kBlack);
		cut->SaveAs(outputfile.c_str());

	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    
}
