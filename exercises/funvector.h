#ifndef __funvector_h_
#define __funvector_h_

template <typename T> struct FunVector
{
    FunVector(int reserved=1)
    {
        grow_factor = 2;
        ptr = NULL;
        sz = 0;
        max_sz = 0;
        alloc(reserved);
    }

    ~FunVector()
    {
        free(ptr);
    }

    T& operator[](int i)
    {
        assert(i < sz);
        return ptr[i];
    }

    const T& operator[](int i) const
    {
        assert(i < sz);
        return ptr[i];        
    }

    void push_back(const T& v)
    {
        if (sz == max_sz)
            alloc();
        assert(sz < max_sz);
        ptr[sz] = v;
        ++sz;
    }

    void pop_back()
    {
        assert(sz > 0);
        --sz;
    }

    T& begin()
    {
        assert(sz > 0);
        return ptr[0];
    }

    T& end()
    {
        assert(sz > 0);
        return ptr[sz-1];
    }

    int size()
    {
        return sz;
    }

private:
    void alloc(int s=-1)
    {
        int oldsize = max_sz;
        max_sz = s < 0 ? oldsize * grow_factor : s;
        ptr = (T*)realloc(ptr, max_sz*sizeof(T));
        if (sz > max_sz)
            sz = max_sz;
    }

    T* ptr;
    int sz;
    int max_sz;
    int grow_factor;
};

#endif

