#ifndef QFR_ENTANGLEMENT_H
#define QFR_ENTANGLEMENT_H

#include <QuantumComputation.hpp>

namespace qc {
    class Entanglement : public QuantumComputation {
    protected:

    public:
        explicit Entanglement(unsigned short nq);
        ~Entanglement() override = default;
    };
}


#endif //QFR_ENTANGLEMENT_H
