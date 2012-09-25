#include <iostream>
#include "thread.h"
int myfun(int i)
{
    std::cout << "myfun received: " << i << std::endl;
    return i*2;
}

class myclass
{
public:

    myclass(int i) : i(i) {}

    int mymember(int k)
    {
        std::cout << "mymember received: " << k << std::endl;
        return i*2;
    }
    int i;

    int operator()(int j)
    {
        std::cerr << "operator received: " << j << std::endl;
        return i*2;
    }

};

int main(int argc, char** argv)
{

    int (*mypt)(int) = &myfun;
    int (&myref)(int) = myfun;
    myclass mc(5);

    // Call to function
    Result<int> r0 = Thread::run(myfun, 0);
    std::cout << "retval:" << r0.value() << std::endl;

    // Call to function pointer
    Result<int> r1 = Thread::run(mypt, 1);
    std::cout << "retval:" << r1.value() << std::endl;

    // Call to function reference
    Result<int> r2 = Thread::run(myref, 2);
    std::cout << "retval:" << r2.value() << std::endl;

    // Call to functor
    Result<int> r3 = Thread::run<int>(mc, 3);
    std::cout << "retval:" << r3.value() << std::endl;

    // Call to member function pointer
    Result<int> r4 = Thread::run(&mc, &myclass::mymember, 4);
    std::cout << "retval:" << r4.value() << std::endl;

    return 0;
}
