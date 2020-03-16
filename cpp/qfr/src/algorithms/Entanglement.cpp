#include "algorithms/Entanglement.hpp"

namespace qc {
    Entanglement::Entanglement(unsigned short nq) : QuantumComputation(nq) {
        emplace_back<StandardOperation>(nqubits, 0, H);

        for(unsigned short i = 1; i < nq; i++) {
            emplace_back<StandardOperation>(nqubits, Control(0,Control::pos), i, X);
        }
    }
}


