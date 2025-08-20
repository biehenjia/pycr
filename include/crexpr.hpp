#pragma once 
#include "crobj.hpp"

class CRexpr final: public CRobj {
    public:

        CRexpr(oc ot, size_t length);
        CRexpr( oc ot, const CRobj& o1);
        CRexpr(oc ot, const CRobj& o1, const CRobj& o2);


        std::unique_ptr<CRobj> add(const CRobj&) const override;
        std::unique_ptr<CRobj> mul(const CRobj&) const override;
        std::unique_ptr<CRobj> pow(const CRobj&) const override;

        std::unique_ptr<CRobj> exp() const override;
        std::unique_ptr<CRobj> ln() const override;
        std::unique_ptr<CRobj> sin() const override; 
        std::unique_ptr<CRobj> cos() const override;

        std::unique_ptr<CRobj> copy() const override;
        void print_tree() const override;
        std::string genCode(size_t parent, size_t index, int place,std::string indent) const override;
        double valueof() const override;
        void shift(size_t i) override final {
            
            for (size_t j = 0; j < isanumber.size(); j++){ 
                operands[isanumber[j]]->shift(i);
                fastvalues[isanumber[j]] = operands[j]->valueof();
            }
        }

        oc optype;

};