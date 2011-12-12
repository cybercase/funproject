#include <iostream>
#include "thread.h"
#include <ctime>

int tt(int i)
{
    std::cerr << "t" << i << "t\n";
    return i*9;
}


int main(int argc, char** argv)
{

    Result<int> r = Thread::run(tt, 1);
    std::cout << "Val:" << r.value() << std::endl;
    std::cout << "OK\n";
    return 0;
}
