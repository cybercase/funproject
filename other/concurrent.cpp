#include <iostream>
#include "thread.h"
#include <ctime>

int tt()
{
    int i = 3, j =3;
    std::cerr << "t" << i*j << "t\n";
    return i*9;
}

class prova
{
    public:
    prova(int i, int j) : i(i), j(j) {std::cerr << "creata\n"; }
    int stampa(int k)
    {
        std::cerr << "stampa: " << i*j*k << "t\n";
        return i+j;
    }
    int i, j;

    int operator()()
    {
        std::cerr << "operator" << i*j << "\n";
        return 1;
    }
};

int main(int argc, char** argv)
{

    Result<int> r = Thread::run(tt);
    prova p(4, 5);
    Result<int> r3 = Thread::run(&p, &prova::stampa, 2);
    Result<int> r2 = Thread::run<int>(prova(7,6));
    r2.value();
    r3.value();
    r.value();
//    std::cout << "Val:" << r.value() << std::endl;
//    std::cout << "Val2:" << r2.value() << std::endl;    
//    std::cout << "OK\n";
    return 0;
}
