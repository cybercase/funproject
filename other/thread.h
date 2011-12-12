#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>
#include <cerrno>
#include <stdexcept>

template <typename T> struct _thread
{
    // This struct will hold the return value of the thread called func.
    
    _thread() : counter(0)
    {
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    }

    ~_thread() { pthread_detach(thd); }

    T result;
    pthread_t thd;
    pthread_attr_t attr;
    volatile int counter;

    int join() { return pthread_join(thd, NULL); }
    
    // Atomic function for add and sub
    int inc() { return __sync_fetch_and_add(&counter, 1); }
    int dec() { if (__sync_fetch_and_sub(&counter, 1) == 0) delete this; }

    private:
    _thread(const _thread&);
    _thread& operator=(const _thread&);
};

template <typename T, typename I> struct _help_st
{
    // This structure holds the pointer to the thread called function
    // and its arguments. It also contains a pointer to
    // the result-holding structure.
    _help_st(_thread<T>* t, T(*f)(I), I a0) : thd(t), 
                                              fun(f),
                                              a0(a0)
                                              { thd->inc(); }
    ~_help_st() { thd->dec(); }

    T(*fun)(I);
    I a0;
    _thread<T>* thd;

    void exec() { thd->result = fun(a0); }

private:
    _help_st(const _help_st&);
    _help_st& operator=(const _help_st&);
};

template <typename S> void* _help_fn(void* v)
{
    // This function has a valid signature to be callen by
    // pthread_create func.
    S* st = (S*)v;
    st->exec();
    delete st;
};

template <typename T> struct Result
{

    Result(_thread<T>* thd) : thd(thd)
    {
        thd->inc();
    }

    Result(const Result<T>& o)
    {
        thd->inc();
        thd = o.thd;
    }

    Result<T>& operator=(const Result& o)
    {
        o.thd->inc();
        thd->dec();
        thd = o.thd;
        return *this;
    }

    T value()
    {
        switch (thd->join())
        {
            case 0:
              break;
            case EINVAL:
              throw std::runtime_error("EINVAL on pthread_join");
              break;
            case EDEADLK:
              throw std::runtime_error("EDEADLK on pthread_join");
              break;
            case ESRCH:
              // Thread already exited 
              break;
        }
        return thd->result;
    }

    ~Result()
    {
        thd->dec();
    }

    _thread<T>* thd;
};


class Thread
{

public:
    template <typename T, typename I> static Result<T> run(T(*fun)(I), I a0)
    {
        
        _thread<T>* mythread = new _thread<T>();

        _help_st<T, I>* h = new _help_st<T, I>(mythread, fun, a0);
        
        pthread_create(&mythread->thd, &mythread->attr,
            _help_fn< _help_st<T, I> >, h);

        return Result<T>(mythread);
    }
};


#endif
