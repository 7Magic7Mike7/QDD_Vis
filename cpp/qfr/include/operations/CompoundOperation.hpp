/*
 * This file is part of IIC-JKU QFR library which is released under the MIT license.
 * See file README.md or go to http://iic.jku.at/eda/research/quantum/ for more information.
 */

#ifndef INTERMEDIATEREPRESENTATION_COMPOUNDOPERATION_H
#define INTERMEDIATEREPRESENTATION_COMPOUNDOPERATION_H

#include "Operation.hpp"

namespace qc {

	class CompoundOperation : public Operation {
	protected:
		std::vector<std::shared_ptr<Operation>> ops{ };
	public:
		explicit CompoundOperation(unsigned short nq) {
			std::strcpy(name, "Compound operation:");
			nqubits = nq;
		}

		template<class T, class... Args>
		auto emplace_back(Args&& ... args) {
			parameter[0]++;
			return ops.emplace_back(std::make_shared<T>(args ...));
		}

		void setNqubits(unsigned short nq) override {
			nqubits = nq;
			for (auto& op:ops) {
				op->setNqubits(nq);
			}
		}

		dd::Edge getDD(std::unique_ptr<dd::Package>& dd, std::array<short, MAX_QUBITS>& line) const override {
			dd::Edge e = dd->makeIdent(0, short(nqubits - 1));
			for (auto& op: ops) {
				e = dd->multiply(op->getDD(dd, line), e);
			}
			return e;
		}

		dd::Edge getInverseDD(std::unique_ptr<dd::Package>& dd, std::array<short, MAX_QUBITS>& line) const override {
			dd::Edge e = dd->makeIdent(0, short(nqubits - 1));
			for (auto& op: ops) { 
				e = dd->multiply(e, op->getInverseDD(dd, line));
			}
			return e;
		}

		dd::Edge getDD(std::unique_ptr<dd::Package>& dd, std::array<short, MAX_QUBITS>& line, std::map<unsigned short, unsigned short>& permutation) const override {
			dd::Edge e = dd->makeIdent(0, short(nqubits - 1));
			for (auto& op: ops) {
				e = dd->multiply(op->getDD(dd, line, permutation), e);
			}
			return e;
		}

		dd::Edge getInverseDD(std::unique_ptr<dd::Package>& dd, std::array<short, MAX_QUBITS>& line, std::map<unsigned short, unsigned short>& permutation) const override {
			dd::Edge e = dd->makeIdent(0, short(nqubits - 1));
			for (auto& op: ops) {
				e = dd->multiply(e, op->getInverseDD(dd, line, permutation));
			}
			return e;
		}

		std::ostream& print(std::ostream& os) const override {
			return print(os, standardPermutation);
		}

		std::ostream& print(std::ostream& os, const std::map<unsigned short, unsigned short>& permutation) const override {
			os << name;
			for (const auto & op : ops) {
				os << std::endl << "\t\t";
				op->print(os, permutation);
			}

			return os;
		}

		bool actsOn(unsigned short i) override {
			for (const auto& op: ops) {
				if(op->actsOn(i))
					return true;
			}
			return false;
		}

		void dumpOpenQASM(std::ofstream& of, const regnames_t& qreg, const regnames_t& creg) const override {
			for (auto& op: ops) { 
				op->dumpOpenQASM(of, qreg, creg);
			}
		}

		void dumpReal(std::ofstream& of) const override {
			for (auto& op: ops) {
				op->dumpReal(of);
			}
		}

		void dumpQiskit(std::ofstream& of, const regnames_t& qreg, const regnames_t& creg, const char *anc_reg_name) const override {
			for (auto& op: ops) {
				op->dumpQiskit(of, qreg, creg, anc_reg_name);
			}
		}
	};
}
#endif //INTERMEDIATEREPRESENTATION_COMPOUNDOPERATION_H
