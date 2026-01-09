#ifndef __STRING_MANIP_FUNCTIONS_HPP__
#define __STRING_MANIP_FUNCTIONS_HPP__

#include <string>
#include <vector>
#include <array>

#include <boost/describe.hpp>

template<class E> struct enum_descriptor
{
	E value;
	char const * name;
};

template<class E, template<class... T> class L, class... T>
	constexpr std::array<enum_descriptor<E>, sizeof...(T)>
describe_enumerators_as_array_impl( L<T...> )
{
	return { { { T::value, T::name }... } };
}

template<class E> constexpr auto describe_enumerators_as_array()
{
	return describe_enumerators_as_array_impl<E>( boost::describe::describe_enumerators<E>() );
}

namespace StringManip{
	std::string StripFileExtension(const std::string&);
	std::string GetFileExtension(const std::string&);
	std::string GetFileBaseName(const std::string&);
	std::string GetFilePath(const std::string&);

	void ParseCalString(const std::string& calstring,std::vector<double>& vals);
	std::string tolower(std::string);
}

#endif
