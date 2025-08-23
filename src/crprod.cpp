#include "crprod.hpp"
#include "crexpr.hpp"
#include "crnum.hpp"
#include "crsum.hpp"

CRprod::CRprod(long long i, size_t l)
{
    length = l;
    operands.resize(l);
    index = i;
    auxiliary.resize(l);
}

std::unique_ptr<CRobj> CRprod::add(const CRobj &target) const
{
    return std::make_unique<CRexpr>(oc::ADD, *this->copy(), *target.copy());
}

std::unique_ptr<CRobj> CRprod::mul(const CRobj &target) const
{
    if (index != target.index)
    {
        std::unique_ptr<CRobj> result = copy();
        std::unique_ptr<CRobj> temp = nullptr;
        if (operands[0]->index > target.index)
        {
            temp = operands[0]->mul(target);
        }
        else
        {
            temp = operands[0]->mul(target);
        }
        result->operands[0] = std::move(temp);
        result->simplify();
        return result;
    }
    else if (auto p = dynamic_cast<const CRprod *>(&target))
    {
        size_t newlength = std::max(length, p->length);
        auto result = std::make_unique<CRprod>(index, newlength);
        for (size_t i = 0; i < newlength; ++i)
        {
            double a = (i < length) ? this->operands[i]->valueof() : 1.0;
            double b = (i < target.length) ? p->operands[i]->valueof() : 1.0;
            result->operands[i] = std::make_unique<CRnum>(a * b);
        }
        result->simplify();
        return result;
    }
    else
    {
        return std::make_unique<CRexpr>(oc::MUL, *this->copy(), *target.copy());
    }
}

std::unique_ptr<CRobj> CRprod::pow(const CRobj &target) const
{

    if (index != target.index)
    {
        auto result = copy();
        std::unique_ptr<CRobj> temp = nullptr;
        for (size_t i = 0; i < length; i++)
        {
            result->operands[i] = operands[i]->pow(target);
        }
        return result;
    }
    else if (auto p = dynamic_cast<const CRsum *>(&target))
    {
        size_t newlength = length + p->length - 1;
        size_t n, m;
        auto result = std::make_unique<CRprod>(index, newlength);
        if (length > p->length)
        {
            m = length;
            n = p->length;
        }
        else
        {
            m = p->length;
            n = length;
        }
        for (size_t i = 0; i < newlength; i++)
        {
            double crs1 = 1.0;
            size_t bound11 = std::max(size_t(0), i - n);
            size_t bound12 = std::min(i, m);

            for (size_t j = bound11; j < bound12 + 1; j++)
            {
                double crs2 = 1.0;
                size_t bound21 = i - j;
                size_t bound22 = std::min(i, n);

                for (size_t k = bound21; k < bound22 + 1; k++)
                {
                    double crt1 = p->operands[k]->valueof() * choose(j, i - k);
                    double crt2 = crt1 * choose(i, j);
                    double crt3 = std::pow(operands[j]->valueof(), crt2);

                    double crt = crs2 * crt3;
                    crs2 = crt;
                }

                double crt4 = crs2 * crs1;
                crs1 = crt4;
            }
            result->operands[i] = std::make_unique<CRnum>(crs1);
        }
        result->simplify();
        return result;
    }
    else
    {
        return std::make_unique<CRexpr>(oc::POW, *this->copy(), *target.copy());
    }
}

void CRprod::simplify()
{
    size_t found = length;
    for (size_t i = length - 1; i > 0; i--)
    {
        if (operands[i]->valueof() == 0)
        {
            found = i - 1;
        }
    }
    // note: will be (a1^a2^...^ak^0^...an) = (1)^...^an = 1
    if (found < length)
    {
        operands.resize(1);
        operands[0] = std::make_unique<CRnum>(1);
    }
    else
    {
        size_t p = 0;
        for (size_t i = 0; i < length; i++)
        {
            while (p < i && operands[p]->valueof() != 1)
            {
                p++;
            }
            if (operands[i]->valueof() != 1)
            {
                std::swap(operands[i], operands[p]);
            }
        }
        if (p == 0)
        {
            operands.clear();
            operands.reserve(1);
            operands.push_back(std::make_unique<CRnum>(1));
            length = 1;
        }
        else
        {
            operands.resize(p);
            length = p;
        }
    }
}

std::unique_ptr<CRobj> CRprod::exp() const
{
    return std::make_unique<CRexpr>(oc::EXP, *this->copy());
}

std::unique_ptr<CRobj> CRprod::ln() const
{
    auto result = std::make_unique<CRsum>(index, length);

    for (size_t i = 0; i < length; i++)
    {
        auto temp = operands[i]->ln();
        result->operands[i] = std::move(temp);
    }
    result->simplify();
    return result;
}

std::unique_ptr<CRobj> CRprod::sin() const
{
    return std::make_unique<CRexpr>(oc::SIN, *this->copy());
}

std::unique_ptr<CRobj> CRprod::cos() const
{
    return std::make_unique<CRexpr>(oc::COS, *this->copy());
}

std::unique_ptr<CRobj> CRprod::copy() const
{
    auto result = std::make_unique<CRprod>(index, length);
    for (size_t i = 0; i < length; i++)
    {
        result->operands[i] = operands[i]->copy();
    }

    if (initialized)
    {
        result->initialized = true;
        result->fastvalues.resize(length);
        result->isnumbers.resize(length);
        for (size_t i = 0; i < length; i++)
        {
            result->fastvalues[i] = fastvalues[i];
            result->isnumbers[i] = isnumbers[i];
        }
        result->isanumber.resize(isanumber.size());
        for (size_t i = 0; i < isanumber.size(); i++)
        {
            result->isanumber[i] = isanumber[i];
        }
    }
    return result;
}

std::unique_ptr<CRobj> CRprod::correctp(size_t nl) const
{
    auto result = copy();
    result->operands.resize(nl);
    for (size_t i = length; i < nl; i++)
    {
        result->operands[i] = std::make_unique<CRnum>(1.0);
    }
    return result;
}

void CRprod::print_tree() const
{
    std::cout << "CRprod" << "[" << valueof() << "]" << "(";
    for (size_t i = 0; i < operands.size(); i++)
    {
        operands[i]->print_tree();
        if (i + 1 < operands.size())
        {
            std::cout << ", ";
        }
    }
    std::cout << ")";
}

std::string CRprod::genCode(size_t parent,long long order,long long place,std::string indent) const
{
    std::string res;

    if (order != index)
    {
        for (size_t i = 0; i < operands.size(); ++i)
        {
            if (!operands[i]->isnumber())
            {
                res += operands[i]->genCode(crposition, order, i, indent);
            }
        }
    }
    else
    {
        size_t n = operands.size();
        res += indent + "for i in range(" + std::to_string(n - 1) + "):\n";

        res += indent + "    " + crprefix + std::to_string(crposition) + "[i]*=" + crprefix + std::to_string(crposition) + "[i+1]\n";
    }

    if (place != -1 && !res.empty())
    {
        res += indent + crprefix + std::to_string(parent) + "[" + std::to_string(place) + "]=" + crprefix + std::to_string(crposition) + "[0]\n";
    }

    return res;
}
