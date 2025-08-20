#pragma once 
#include "crobj.hpp"

class CRtrig final: public CRobj { 
    public:
        CRtrig(int i, oc t, size_t l);

        std::unique_ptr<CRobj> add(const CRobj& target) const override;
        std::unique_ptr<CRobj> mul(const CRobj& target) const override;
        std::unique_ptr<CRobj> pow(const CRobj& target) const override;

        std::unique_ptr<CRobj> exp() const override;
        std::unique_ptr<CRobj> ln() const override;

        std::unique_ptr<CRobj> sin() const override;
        std::unique_ptr<CRobj> cos() const override;

        void simplify() override; 
        std::unique_ptr<CRobj> copy() const override;
        void shift(size_t i) override final{
            if (index != i){ 
                for (size_t j = 0; j < isanumber.size(); j++){ 
                    operands[isanumber[j]]->shift(i);
                    fastvalues[isanumber[j]] = operands[j]->valueof();
                }
            } else { 
                double r1, r2, r3, r4, z;
                size_t t = length/2;
                for (size_t j = 0; j < t-1; j++){ 
                    r1 = fastvalues[j] * fastvalues[j+t+1];
                    r2 = fastvalues[j+t] * fastvalues[j+1];
                    z = r1 + r2;
                    r3 = fastvalues[j+t] * fastvalues[j+t+1];
                    r4 = fastvalues[j] * fastvalues[j+1];
                    fastvalues[j+t] = r3-r4;
                    fastvalues[j] = z;
                }
            }
            
}
        // double initialize();
        double valueof() const;
        void print_tree() const override;
        std::string genCode(size_t parent, size_t index, int place,std::string indent) const override;

        oc trigtype;
        std::unique_ptr<CRobj> correctt(size_t nl) const;
        size_t index;
};