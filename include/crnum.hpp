#pragma once 
#include "crobj.hpp"

class CRnum final: public CRobj {
    public:
        CRnum(double v);
        std::unique_ptr<CRobj> add(const CRobj& target) const override;
        std::unique_ptr<CRobj> mul(const CRobj& target)  const override;
        std::unique_ptr<CRobj> pow(const CRobj& target) const  override;

        std::unique_ptr<CRobj> exp() const override;
        std::unique_ptr<CRobj> ln() const override;
        std::unique_ptr<CRobj> sin()  const override;
        std::unique_ptr<CRobj> cos() const override;

        void simplify() override;
        std::unique_ptr<CRobj> copy() const override;

        double initialize(); 
        double valueof() const;
        bool isnumber() const override;
        void print_tree() const override;
        std::string genCode(size_t parent, size_t index, int place,std::string indent) const override;

        void shift(size_t index) override final {
            return;
        }
        double value;

};