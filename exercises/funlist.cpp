#include <iostream>
#include <cassert>
#include <vector>
#include <list>
#include <cstdlib>
#include <ctime>
#include "funlist.h"
#include "funvector.h"

struct Complex
{
    Complex() {}
    Complex(const double& r) : real(r), img(0) {}
    Complex(const double& r, const double& i) : real(r), img(i) {}
    double real;
    double img;
};

std::ostream& operator << (std::ostream& o, const Complex& c)
{
    return o << c.real << "+i" << c.img;
}

int main(int argc, char** argv)
{
    std::cerr << sysconf( _SC_PAGESIZE) << std::endl;
    int N = 1000000;

    {
    std::list<Complex> lst;        
    clock_t start = clock();
    for (int i=0; i<N; ++i)
        lst.push_back(i);
    clock_t stop = clock();
    std::cerr << "std::list: " << double(stop-start)/CLOCKS_PER_SEC << std::endl;
    }

    {
    FunList<Complex> flist;
    clock_t start = clock();
    for (int i=0; i<N; ++i)
        flist.push_back(i);
    clock_t stop = clock();
    std::cerr << "FunList: " << double(stop-start)/CLOCKS_PER_SEC << std::endl;
    }

    {
    std::vector<Complex> vec;
    clock_t start = clock();
    for (int i=0; i<N; ++i)
        vec.push_back(i);
    clock_t stop = clock();
    std::cerr << "std::vector: " << double(stop-start)/CLOCKS_PER_SEC << std::endl;
    }

    {
    FunVector<Complex> fvec;        
    clock_t start = clock();
    for (int i=0; i<N; ++i)
        fvec.push_back(i);
    clock_t stop = clock();
    std::cerr << "FunVector: " << double(stop-start)/CLOCKS_PER_SEC << std::endl;
    }
}

/*
int main(int argc, char** argv)
{
    FunList<Complex> dlist;
    for (int i=0; i<10; ++i)
        dlist.push_back(Complex(i));
    dlist.pop_front();
    dlist.push_front(90);
    dlist.pop_back();
    dlist.pop_back();
    dlist.pop_back();
    dlist.dump();

    std::cerr << "MyDump" << std::endl;
    
    FunList<Complex>::LI* ite = dlist.begin();
    while(ite != dlist.end())
    {
        //ite->val = 10.0;
        std::cerr << "VAL: " << ite->val << std::endl;
        ite = ite->next;
    }
    return 1;
}
*/

