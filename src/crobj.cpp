#include "crobj.hpp"
#include <iostream>



CRobj::CRobj(size_t l)
{
    length = l;
    operands.resize(l);
}

double CRobj::valueof() const
{
    if (initialized)
    {
        return fastvalues[0];
    }
    return 0;
}

bool CRobj::isnumber() const
{
    return false;
}

double CRobj::initialize()
{
    if (initialized)
    {
        return valueof();
    }
    initialized = true;
    fastvalues.resize(length, 0);
    isnumbers.resize(length, false);

    for (size_t i = 0; i < length; i++)
    {
        if (operands[i]->isnumber())
        {
            isanumber.push_back(i);
        }
        isnumbers[i] = operands[i]->isnumber();
        fastvalues[i] = operands[i]->initialize();
    }
    return valueof();
}

void CRobj::simplify()
{
    for (size_t i = 0; i < length; i++)
    {
        operands[i]->simplify();
    }
}

void CRobj::shift(size_t index)
{
    return;
}

std::string CRobj::prepare(CRobj &root)
{
    root.crcount++;
    crposition = root.crcount;

    std::string res;
    std::string temp = crprefix + std::to_string(crposition) + "=[";

    for (size_t i = 0; i < operands.size(); ++i)
    {
        if (!operands[i]->isnumber())
        {
            res += operands[i]->prepare(root);
        }
        temp += std::to_string(fastvalues[i]);
        if (i + 1 < operands.size())
            temp.push_back(',');
    }

    temp += "]\n";
    if (!operands.empty())
        res += temp;

    return res;
}

