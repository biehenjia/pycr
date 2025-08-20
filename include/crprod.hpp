#pragma once
#include "crobj.hpp"

class CRprod final:public CRobj {
    public: 
        CRprod(int i, size_t l);
        
        std::unique_ptr<CRobj> add(const CRobj& target) const override;
        std::unique_ptr<CRobj> mul(const CRobj& target) const override;
        std::unique_ptr<CRobj> pow(const CRobj& target) const override;

        //todo;
        std::unique_ptr<CRobj> exp() const override;
        std::unique_ptr<CRobj> ln() const override;

        std::unique_ptr<CRobj> sin() const override;
        std::unique_ptr<CRobj> cos() const override;

        void simplify() override;
        void shift(size_t i) override final { 
            if (index > i){
                for (size_t j = 0; j < isanumber.size(); j++){ 
                    operands[isanumber[j]]->shift(i);
                    fastvalues[isanumber[j]] = operands[j]->valueof();
                }
            } else { 
                for (size_t j = 0; j < length-1; j++){
                    fastvalues[j] *= fastvalues[j+1 ];
                }
            }
        }
        void print_tree() const override;
        std::string genCode(size_t parent, size_t index, int place,std::string indent) const override;



        std::unique_ptr<CRobj> copy() const override;
        std::unique_ptr<CRobj> correctp(size_t nl) const;
};

