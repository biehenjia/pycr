#include "crnum.hpp"
#include "crsum.hpp"
#include "crexpr.hpp"
#include "crprod.hpp"

CRnum::CRnum(double v){
    value = v;
    index = -1;
}

// assume invariant
std::unique_ptr<CRobj> CRnum::add(const CRobj& target) const  {
    auto p = dynamic_cast<const CRnum*>(&target);
    return std::make_unique< CRnum>(this->value + p->value);
}

std::unique_ptr<CRobj> CRnum::mul(const CRobj& target) const { 
    auto p = dynamic_cast<const CRnum*>(&target);
    return std::make_unique< CRnum>(this->value * p->value);
}

// noncommutative
std::unique_ptr<CRobj> CRnum::pow(const CRobj& target) const {
    if (auto p= dynamic_cast<const CRnum*>(&target)){
        return std::make_unique< CRnum>(std::pow(this->value,p->value));
    } else if (auto p = dynamic_cast<const CRsum*>(&target)){
        auto result = std::make_unique< CRprod>(p->index, p->length);
        for (size_t i = 0; i< p->length; i++){ 
            result->operands[i] = std::make_unique< CRnum>(std::pow(this->value,p->operands[i]->valueof()));
        }
        return result;
    }
    return std::make_unique< CRexpr>(oc::POW, *this->copy(), *target.copy());
}

std::unique_ptr<CRobj> CRnum::ln() const { 
    return std::make_unique< CRnum>(std::log(value));
}

std::unique_ptr<CRobj> CRnum::sin() const { 
    return std::make_unique< CRnum>(std::sin(value));
}

std::unique_ptr<CRobj> CRnum::cos() const { 
    return std::make_unique< CRnum>(std::cos(value));
}

std::unique_ptr<CRobj> CRnum::copy() const{
    return std::make_unique< CRnum>(this->value);
}

bool CRnum::isnumber() const {
    return true;
}

std::unique_ptr<CRobj> CRnum::exp() const { 
    return std::make_unique< CRnum>(std::exp(value ));
}

double CRnum::initialize(){
    return value;
}

double CRnum::valueof() const { 
    return value; 
}

void CRnum::simplify() {
    return;
}



void CRnum::print_tree() const { 
    std::cout << "CRnum("<<value<<")";
}

std::string CRnum::genCode(size_t parent, size_t order, int place,std::string indent) const {
    return "";
}