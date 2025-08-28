#include "_ast.hpp"
#include "crsum.hpp"
#include "crnum.hpp"
#include "chrono"
/*
in python, we construct the AST, and each symbolic node (variable) is initialized with
the proper start and step and index. Then, we call crinit that passes the number of evaluations
needed for each one
*/

std::unique_ptr<CRobj> m1 = std::make_unique<CRnum>(-1);

std::unique_ptr<CRobj> ASTvar::crmake()
{
    return std::make_unique<CRsum>(index, start, step);
}

std::unique_ptr<CRobj> ASTnum::crmake()
{
    return std::make_unique<CRnum>(value);
}

std::unique_ptr<CRobj> ASTbin::crmake()
{
    
    // std::cout<<"crmake on astbin called \n";
    std::unique_ptr<CRobj> result;
    auto crleft = left->crmake();
    auto crright = right->crmake();
    // std::cout<<crleft->index<<" "<<crright->index<<"\n";
    switch (optype)
    {
    case bt::ADD:
        if (crleft->index > crright->index)
        {
            result = crleft->add(*crright);
        }
        else
        {
            result = crright->add(*crleft);
        }
        break;
    case bt::MUL:
        if (crleft->index > crright->index)
        {
            result = crleft->mul(*crright);
        }
        else
        {
            result = crright->mul(*crleft);
        }
        break;
    case bt::DIV:
        result = (crleft->mul(*crright->pow(*m1)));
        break;
    case bt::SUB:
        if (crleft->index > crright->index)
        {
            result = crleft->add((*crright->mul(*m1)));
        }
        else
        {
            result = (crright->mul(*m1))->add(*crleft);
        }
        break;
    case bt::POW:
        result = (crleft->pow(*crright));
        break;
    }
    return result;
}

std::unique_ptr<CRobj> ASTun::crmake()
{
    // std::cout<<"making AST unary \n";
    std::unique_ptr<CRobj> result;
    auto crleft = left->crmake();
    switch (optype)
    {
    case ut::COS:
        result = crleft->cos();
        break;
    case ut::SIN:
        result = crleft->sin();
        break;
    case ut::EXP:
        result = crleft->exp();
        break;
    case ut::LN:
        result = crleft->ln();
        break;
    }
    return result;
}

void ASTnode::crinit(std::vector<size_t> p)
{
    params = p;
    cr = crmake();
    cr->initialize();
    cr->print_tree();
    size_t k = 1;

    for (auto v : p)
    {
        k *= v;
    }
    result.clear();
    result.resize(k);
    writeindex = 0;
}

std::string ASTnode::crgen()
{
    std::string res;
    std::string indent;
    std::string expr = "0";

    for (size_t i = 0; i < params.size(); ++i)
    {
        expr = "[" + expr + " for _ in range(" + std::to_string(params[i]) + ")]";
    }

    res = "results = " + expr + "\n";
    res += cr->prepare(*cr);

    std::string base = "base = [";
    std::string delim = ",";
    for (size_t i = 0; i < cr->length; ++i)
    {
        if (i == cr->length - 1)
        {
            delim.clear();
        }
        base += std::to_string(cr->fastvalues[i]) + delim;
    }
    base += "]\n";
    //res += base;

    std::string indexpos = "results";
    for (size_t i = 0; i < params.size(); ++i)
    {
        indexpos += "[_" + std::to_string(i) + "]";
    }
    for (size_t i = 0; i < params.size(); ++i)
    {
        res += indent + cr->crprefix + std::to_string(cr->crposition) + " = base[:]\n";

        res += indent + "for _" + std::to_string(i) + " in range(" + std::to_string(params[i]) + "):\n";

        indent += "    ";

        if (i + 1 == params.size())
        {
            res += indent + indexpos + " = " + cr->crprefix + std::to_string(cr->crposition) + "[0]\n";
        }
        res += cr->genCode(0, i, -1, indent);
        res += "\n";
    }

    return res;
}



void ASTnode::_creval()
{
    using Clock = std::chrono::steady_clock;
    using Millis = std::chrono::duration<double, std::milli>;

    double total_shift = 0.0;
    double total_copy  = 0.0;
    double total_push  = 0.0;
    double total_eval = 0.0;

    size_t n = params.size();
    std::vector<size_t> ind(n,0);
    std::vector<std::unique_ptr<CRobj>> crs;
    // fast values pointers
    std::vector<double*> fvptrs;


    crs.reserve(n);

    // build initial copies
    crs.push_back(cr->copy());
    for (size_t i = 1; i < n; ++i) {
        auto t0 = Clock::now();
        crs.push_back(crs[i-1]->copy());
        total_copy += Millis( Clock::now() - t0 ).count();
    }

    size_t paramsize = n - 1;

    

    double* out = result.data();
    double val;

    double* fastvalue = crs.back()->fastvalues.data();
    auto loop_start = Clock::now();
    while (true) {
        for (size_t i = 0; i < crs[paramsize]->fastvalues.size(); i++){
            std::cout<<crs[paramsize]->fastvalues[i]<<" ";
        } std::cout<<"\n";
        // 1) push_back timing
        for (size_t i = 0; i < n; i++){
            crs[i]->print_tree();
            std::cout<<"\n";
        }
        {
            //auto t0 = Clock::now();
            val = fastvalue[0];
            //total_eval += Millis(Clock::now() - t0).count();
        }

        {
            //auto t0 = Clock::now();
            *out++ =  val;
            //total_push += Millis( Clock::now() - t0 ).count();
        }

        // 2) shifting at the last index
        {
            //auto t1 = Clock::now();
            crs[paramsize]->shift(paramsize);
            //total_shift += Millis( Clock::now() - t1 ).count();
        }

        // 3) advance the odometer
        long long i = paramsize;
        while (i >= 0) {
            ind[i]++;
            if (ind[i] < params[i]) {
                // copy forward for the “roll-over” indices
                for (size_t j = i+1; j < n; ++j) {
                    auto t2 = Clock::now();
                    crs[j] = crs[j-1]->copy();
                    total_copy += Millis( Clock::now() - t2 ).count();
                    ind[j] = 0;
                }
                fastvalue = crs.back()->fastvalues.data();
                break;
            }

            // reset this digit
            ind[i] = 0;
            if (i > 0) {
                // shifting the next-up digit
                //auto t3 = Clock::now();
                crs[i-1]->shift(i-1);
                std::cout<<"shift index: "<<i<<"\n";
                //total_shift += Millis( Clock::now() - t3 ).count();
                
                // then copy it forward
                //auto t4 = Clock::now();
                crs[i] = crs[i-1]->copy();
                //total_copy += Millis( Clock::now() - t4 ).count();
            }
            --i;
        }

        if (i < 0) break;
    }

    double total_loop = Millis( Clock::now() - loop_start ).count();

    std::cout
        << "Total loop time: " << total_loop  << " ms\n"
        << "  valueof():     " << total_eval << " ms\n"
        << "  push_back:     " << total_push  << " ms\n"
        << "  shifting:      " << total_shift << " ms\n"
        << "  copying:       " << total_copy  << " ms\n";
}

std::vector<double> ASTnode::creval()
{
    _creval();
    return result;
}

void ASTnode::view()
{
    // std::cout<<"astnode\n";
    left->view();
    right->view();
}

void ASTvar::view()
{
    std::cout << "astvar(" << index << ")\n";
}

void ASTnum::view()
{
    std::cout << "astnum(" << value << ")\n";
}

void ASTun::view()
{
    std::cout << "astun\n";
    left->view();
}

void ASTbin::view()
{
    std::cout << "astbin\n";
    left->view();
    right->view();
}
