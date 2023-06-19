#ifndef __ARG_VALUE_HPP__
#define __ARG_VALUE_HPP__

#include <string>

template<class T>
class ArgValue{
	public:
		ArgValue(std::string s,std::string l,std::string d,T v){
			sname = s;
			lname = l;
			soptname = "-"+s;
			loptname = "--"+l;
			description = d;
			value = new T(v);
			initialized = false;
		}
		ArgValue(std::string s,std::string l,std::string d,T& v){
			sname = s;
			lname = l;
			soptname = "-"+s;
			loptname = "--"+l;
			description = d;
			value = new T(v);
			initialized = false;
		}
		T* GetValue(){
			return value;
		}
		bool MatchesShortName(std::string& n){
			return soptname.compare(n) == 0;
		}
		bool MatchesLongName(std::string& n){
			return loptname.compare(n) == 0;
		}
		void UpdateValue(bool v){
			if( value != nullptr )
				delete value;
			value = new bool(v);
			initialized = true;
		}
		void UpdateValue(T& v){
			if( value != nullptr )
				delete value;
			value = new T(v);
			initialized = true;
		}
		std::string GetShortName(){
			return sname;
		}
		std::string GetLongName(){
			return lname;
		}
		std::string GetShortOptName(){
			return soptname;
		}
		std::string GetLongOptName(){
			return loptname;
		}

		std::string GetDescription(){
			return description;
		}
		bool IsDefault() const{
			return !initialized;
		}
		~ArgValue(){
			if( value != nullptr )
				delete value;
		}
	private:
		T* value;
		std::string sname;
		std::string lname;
		std::string soptname;
		std::string loptname;
		std::string description;
		bool initialized;
};


#endif
