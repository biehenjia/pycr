#include "crtrig.hpp"
#include "crsum.hpp"
#include "crexpr.hpp"
#include "crnum.hpp"
#include "crprod.hpp"

CRtrig::CRtrig(long long i, oc t, size_t l)
{
    length = l;
    trigtype = t;
    operands.resize(length);
    index = i;
}

std::unique_ptr<CRobj> CRtrig::copy() const
{
    auto result = std::make_unique<CRtrig>(index, trigtype, length);
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

std::unique_ptr<CRobj> CRtrig::add(const CRobj &target) const
{
    return std::make_unique<CRexpr>(oc::ADD, *this->copy(), *target.copy());
}

std::unique_ptr<CRobj> CRtrig::pow(const CRobj &target) const
{
    return std::make_unique<CRexpr>(oc::POW, *this->copy(), *target.copy());
}

std::unique_ptr<CRobj> CRtrig::mul(const CRobj &target) const
{
    if (index != target.index)
    {
        if (trigtype == oc::SIN || trigtype == oc::COS)
        {
            auto result = copy();

            if (operands[0]->index > target.index)
            {
                result->operands[0] = operands[0]->mul(target);
            }
            else
            {
                result->operands[0] = target.mul(*operands[0]);
            }
            if (operands[length / 2]->index > target.index)
            {
                result->operands[length / 2] = operands[length / 2]->mul(target);
            }
            else
            {
                result->operands[length / 2] = target.mul(*operands[0]);
            }
            result->simplify();
            return result;
        }
    }
    else if (auto p = dynamic_cast<const CRprod *>(&target))
    {
        auto result = std::make_unique<CRtrig>(index, trigtype, length);
        size_t L;
        std::vector<CRobj> o1, o2;
        std::unique_ptr<CRobj> c = nullptr;
        std::unique_ptr<CRobj> t = nullptr;
        if (length / 2 > p->length)
        {
            const auto &o1 = operands;
            c = p->correctp(length / 2);
            const auto &o2 = c->operands;
            size_t L = length / 2;
        }
        else if (length / 2 < p->length)
        {
            t = correctt(p->length);
            const auto &o1 = t->operands;
            const auto &o2 = p->operands;
            size_t L = p->length;
        }
        else
        {
            const auto &o1 = operands;
            const auto &o2 = p->operands;
            size_t L = length / 2;
        }
        result->operands.resize(L * 2);
        for (size_t i = 0; i < L; i++)
        {
            if (o1[i].index > o2[i].index)
            {
                result->operands[i] = o1[i].mul(o2[i]);
            }
            else
            {
                result->operands[i + L] = o1[i + L].mul(o2[i]);
            }
        }
        result->length = 2 * L;
        result->simplify();
        return result;
    }
    else
    {
        return std::make_unique<CRexpr>(oc::MUL, *this->copy(), *target.copy());
    }
    return nullptr;
}

std::unique_ptr<CRobj> CRtrig::correctt(size_t nl) const
{
    auto result = copy();
    result->operands.resize(nl * 2);
    for (size_t i = 0; i < length / 2; i++)
    {
        result->operands[i] = operands[i + length / 2]->copy();
    }
    for (size_t i = length / 2; i < nl * 2; i++)
    {
        result->operands[i] = std::make_unique<CRnum>(0.0);
        result->operands[i + nl] = std::make_unique<CRnum>(1.0);
    }
    return result;
}

double CRtrig::valueof() const
{
    double result = 0;
    switch (trigtype)
    {
    case oc::SIN:
        result = fastvalues[0];
        break;
    case oc::COS:
        result = fastvalues[length / 2];
        break;
    case oc::TAN:
        result = fastvalues[0] / fastvalues[length / 2];
        break;
    case oc::COT:
        result = fastvalues[length / 2] / fastvalues[0];
        break;
    }
    return result;
}

std::unique_ptr<CRobj> CRtrig::exp() const
{
    return std::make_unique<CRexpr>(oc::EXP, *this->copy());
}

std::unique_ptr<CRobj> CRtrig::ln() const
{
    return std::make_unique<CRexpr>(oc::LN, *this->copy());
}

std::unique_ptr<CRobj> CRtrig::sin() const
{
    return std::make_unique<CRexpr>(oc::SIN, *this->copy());
}

std::unique_ptr<CRobj> CRtrig::cos() const
{
    return std::make_unique<CRexpr>(oc::COS, *this->copy());
}

// todo
void CRtrig::simplify()
{
}

void CRtrig::print_tree() const
{
    std::cout << "CRtrig" << "[" << valueof() << "]" << "(";

    std::cout << ")";
}

std::string CRtrig::genCode(size_t parent, long long order, long long place, std::string indent) const
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
        size_t t = operands.size() / 2;
        std::string arr = crprefix + std::to_string(crposition);
        res += indent + "for j in range(" + std::to_string(t - 1) + "):\n";
        res += indent + "    r1 = " + arr + "[j] * " + arr + "[j+" + std::to_string(t) + "+1]\n";
        res += indent + "    r2 = " + arr + "[j+" + std::to_string(t) + "] * " + arr + "[j+1]\n";
        res += indent + "    z  = r1 + r2\n";
        res += indent + "    r3 = " + arr + "[j+" + std::to_string(t) + "] * " + arr + "[j+" + std::to_string(t) + "+1]\n";
        res += indent + "    r4 = " + arr + "[j] * " + arr + "[j+1]\n";
        res += indent + "    " + arr + "[j+" + std::to_string(t) + "] = r3 - r4\n";
        res += indent + "    " + arr + "[j] = z\n";
    }

    if (place != -1 && !res.empty())
    {
        res += indent + crprefix + std::to_string(parent) + "[" + std::to_string(place) + "]=" + crprefix + std::to_string(crposition) + "[0]\n";
    }

    return res;
}
