/** \file set2root.hpp
 * \brief A program to convert a pixie16 binary .set file into a root file.
 *
 * This program reads a pixie16 dsp variable file (.var) and a binary .set
 * file and outputs a .root file containing the names of the pixie16
 * parameters and their values.
 *
 * Origonally written by C. R. Thornsberry and S. V. Paulauskas in 2016 for PAASS
 * \author T.T. King, C. R. Thornsberry
 * \date March 13th, 2026
 */
#ifndef SET2ROOT_HPP
#define SET2ROOT_HPP

#include <vector>
#include <string.h>
#include <cmath>

#ifdef USE_ROOT_OUTPUT
class TFile;
#endif

namespace IeeeStandards {
	/// This function converts an IEEE Floating Point number into a standard
	///  decimal format. This function was stolen almost verbatim from
	///  utilities.c provided by XIA. This data format is used by XIA to store
	///  both TAU and the Baseline. Magic numbers abound since we're
	///  literally following a prescription on how this information is
	///  stored.
	/// https://en.wikipedia.org/wiki/IEEE_floating_point#IEEE_754-2008
	///@param[in] IeeeFloatingNumber : The IEEE Floating point number that we
	///  want to convert to decimal
	///@return The decimal number that's been decoded from the input.
	inline double IeeeFloatingToDecimal(
		const unsigned int& IeeeFloatingNumber) {
		double result;
		short signbit = (short)(IeeeFloatingNumber >> 31);
		short exponent =
			(short)((IeeeFloatingNumber & 0x7F800000) >> 23) - 127;
		double mantissa =
			1.0 + (double)(IeeeFloatingNumber & 0x7FFFFF) / pow(2.0, 23.0);
		if (signbit == 0)
			result = mantissa * pow(2.0, (double)exponent);
		else
			result = -mantissa * pow(2.0, (double)exponent);
		return result;
	}
} // namespace IeeeStandards

class parameter {
public:
	std::vector<unsigned int> values;

	parameter()
		: name()
		, offset() {
	}

	parameter(const std::string& name_, const unsigned int& offset_)
		: name(name_)
		, offset(offset_) {
	}

	std::string getName() {
		return name;
	}

	unsigned int getOffset() {
		return offset;
	}

#ifdef USE_ROOT_OUTPUT
	bool write(TFile* f_, const std::string& dir_ = "");
#endif

	std::string print();

private:
	std::string name; /// The name of the pixie16 parameter.
	unsigned int offset; /// The offset of this parameter in the .set file (in words).
};

#endif
