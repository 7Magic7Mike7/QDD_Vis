/*
 * This file is part of IIC-JKU QFR library which is released under the MIT license.
 * See file README.md or go to http://iic.jku.at/eda/research/quantum/ for more information.
 */

#include "operations/Operation.hpp"

namespace qc {
	std::map<unsigned short, unsigned short> Operation::standardPermutation = Operation::create_standard_permutation();

	void Operation::setLine(std::array<short, MAX_QUBITS>& line, const std::map<unsigned short, unsigned short>& permutation) const {
		for(auto target: targets) {
			#if DEBUG_MODE_OPERATIONS
			std::cout << "target = " << target << ", perm[target] = " << permutation.at(target) << std::endl;
			#endif

			line[permutation.at(target)] = LINE_TARGET;
		}
		for(auto control: controls) {
			#if DEBUG_MODE_OPERATIONS
			std::cout << "control = " << control.qubit << ", perm[control] = " << permutation.at(control.qubit) << std::endl;
			#endif

			line[permutation.at(control.qubit)] = control.type == Control::pos? LINE_CONTROL_POS: LINE_CONTROL_NEG;
		}
	}

	void Operation::resetLine(std::array<short, MAX_QUBITS>& line, const std::map<unsigned short, unsigned short>& permutation) const {
		for(auto target: targets) {
			line[permutation.at(target)] = LINE_DEFAULT;
		}
		for(auto control: controls) {
			line[permutation.at(control.qubit)] = LINE_DEFAULT;
		}
	}

    std::ostream& Operation::print(std::ostream& os) const {
		return print(os, standardPermutation);
	}

	std::ostream& Operation::print(std::ostream& os, const std::map<unsigned short, unsigned short>& permutation) const {
		const auto prec_before = std::cout.precision(20);

		os << std::setw(4) << name << "\t";
		std::array<short, MAX_QUBITS> line{};
		line.fill(-1);
		setLine(line);

		for (const auto& physical_qubit: permutation) {
			unsigned short physical_qubit_index = physical_qubit.first;
			if (line[physical_qubit_index] == LINE_DEFAULT) {
				os << "|\t";
			} else if (line[physical_qubit_index] == LINE_CONTROL_NEG) {
				os << "\033[31m" << "c\t" << "\033[0m";
			} else if (line[physical_qubit_index] == LINE_CONTROL_POS) {
				os << "\033[32m" << "c\t" << "\033[0m";
			} else {
				os << "\033[1m\033[36m" << name[0] << name[1] << "\t\033[0m";
			}
		}

		bool isZero = true;
		for (size_t i = 0; i < MAX_PARAMETERS; ++i) {
			if (parameter[i] != 0.L)
				isZero = false;
		}
		if (!isZero) {
			os << "\tp: (";
			CN::printFormattedReal(os, parameter[0]);
			os << ") ";
			for (size_t j = 1; j < MAX_PARAMETERS; ++j) {
				isZero = true;
				for (size_t i = j; i < MAX_PARAMETERS; ++i) {
					if (parameter[i] != 0.L)
						isZero = false;
				}
				if (isZero) break;
				os << "(";
				CN::printFormattedReal(os, parameter[j]);
				os << ") ";
			}
		}

		std::cout.precision(prec_before);

		return os;
	}

}
