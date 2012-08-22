#ifndef ENABLE_DEBUG_TYPES
#warning ENABLE_DEBUG_TYPES is not defined. Some tests will fail.
#endif

//
// Make sure to use ThrowPolicy for these tests
//
#ifdef DEBUG_TYPES_POLICY
#undef DEBUG_TYPES_POLICY
#endif
#define DEBUG_TYPES_POLICY ThrowPolicy

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <vector>

#include "debug_ptr.h"

typedef_pointer(int*, int_p);
typedef_array(int_p*, int_pa);
typedef_array(int_pa*, int_paa);

// TEST 0
//
// Allocate, fill and free a 2 dimensional array of pointers
void test0_inner(bool free) {
    const int N = 10; // columns
    const int M = 10; // rows
    
    int_paa matrix = new int_pa[M]; // Allocates rows
    for (int i = 0; i < M; ++i) {
        matrix[i] = new int_p[N]; // Allocates columns for each row
        for (int j=0; j < N; ++j) {
            int_pa tmp = matrix[i];
            tmp[j] = new int(i*M+j); // Allocates an element
        }
    }

    for (int i = 0; i < M; ++i) {
        for (int j=0; j < N; ++j) {
            delete matrix[i][j]; // Frees elements
        }
        delete [] matrix[i]; // Frees all the columns of a row
    }
    if (free)
        delete [] matrix; // Frees all the rows
}
void test0_ok () {
    test0_inner(true);
}
void test0_bad () {
    test0_inner(false);
}

// TEST 1
//
// Shows how to get a standard pointer from a debug_ptr, and
// how to manage an external deletion

typedef_array(char*, char_a);
void copy_and_free(char* dest, const char* src) {
    strcpy(dest, src);
    delete [] dest;
}
void test1_ok () {
    char_a s0 = new char[10];
    copy_and_free(&s0[0], "1234");

    // Since char[10] inside s0 was deleted by copy_and_free function,
    // we need to mark s0 as deleted by calling debug_types::reset
    debug_types::reset(s0);
}
void test1_bad () {
    char_a s0 = new char[10];
    copy_and_free(&s0[0], "1234");
}


// TEST 2
//
// Using debug_ptr as functions argument
template <typename T> struct Complex {
    Complex() : r_(0), i_(0) {}
    Complex(const T& r, const T& i) : r_(r), i_(i) {}
    T r_;
    T i_;
};
typedef_pointer(Complex<double>*, complex_p);

void swap_complex(const complex_p& l, const complex_p& r) {
    Complex<double> tmp = *l;
    *l = *r;
    *r = tmp;
};
void fill_complex(const complex_p& empty) {
    empty->r_ = 9;
    empty->i_ = 0;
}
void new_complex(complex_p* dest) {
    if (dest != NULL)
        *dest = new Complex<double>;
}
void test2_ok() {
    complex_p dest;
    new_complex(&dest);
    fill_complex(dest);
    delete dest;
}
void test2_bad() {
    complex_p dest;
    new_complex(&dest);
    fill_complex(dest);
}

// TEST 3
//
// Using debug_ptr as class member
template <typename T> class ComplexContainer {
public:
    typedef_pointer(Complex<T>*, inner_complex_p);

    ComplexContainer(inner_complex_p cp) : cplx_(cp) {}
    ~ComplexContainer() { if (debug_types::address_of(cplx_) != NULL) delete cplx_; }

    void correct_assign(inner_complex_p p) {
        delete cplx_;
        cplx_ = p;
    }
    void wrong_assign(inner_complex_p p) {
        cplx_ = p;
    }
private:
    ComplexContainer(const ComplexContainer&);
    ComplexContainer& operator=(const ComplexContainer&);

    inner_complex_p cplx_;
};
void test3_ok() {
    ComplexContainer<int> container(new Complex<int>(1,2));
    container.correct_assign(new Complex<int>(0,0));
}
void test3_bad() {
    ComplexContainer<int> container(new Complex<int>(1,2));
    container.wrong_assign(new Complex<int>(0,0));
}

// TEST4
// 
// Sorting debug_ptr has the same effect of sorting standard pointers
void test4_ok() {
    const int N=100;
    std::vector<int*> pvec;
    std::vector<int_p> dpvec;
    for (int i = 0; i < N; ++i) {
        int* tmp = new int(N-i);
        pvec.push_back(tmp);
        dpvec.push_back(tmp);
    }
    std::random_shuffle(pvec.begin(), pvec.end());
    std::random_shuffle(dpvec.begin(), dpvec.end());
    std::sort(pvec.begin(), pvec.end());
    std::sort(dpvec.begin(), dpvec.end());
    for (int i = 0; i < N; ++i) {
        if (debug_types::address_of(dpvec[i]) != pvec[i])
            break; 

        delete pvec[i];
        debug_types::reset(dpvec[i]); // because it deleted just the line above
    }
}

// Run test utility function
void exec_test( void (*success_fn)(void), void (*fail_fn)(void), const char* id ) {
    if (success_fn) {
        try {
            (*success_fn)();
            std::cerr << id << "_ok: PASSED" << std::endl; 
        } catch (std::runtime_error&) {
            std::cerr << id << "_ok: FAILED" << std::endl;
        }
    }
    if (fail_fn) {
        try {
            (*fail_fn)();
            std::cerr << id << "_bad: FAILED" << std::endl; 
        } catch (std::runtime_error&) {
            std::cerr << id << "_bad: PASSED" << std::endl;
        }
    }
    std::cerr << std::endl;
}

//
// RUN ALL THE TESTS
//
int main(int argc, char** argv) {
    exec_test(test0_ok, test0_bad, "test0");
    exec_test(test1_ok, test1_bad, "test1");
    exec_test(test2_ok, test2_bad, "test2");
    exec_test(test3_ok, test3_bad, "test3");
    exec_test(test4_ok, NULL, "test4");
}
