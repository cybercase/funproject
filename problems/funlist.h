#ifndef __funlist_h_
#define __funlist_h_
#include <cassert>

#define DEFAULT_SIZE 1024

template <typename T, int S=DEFAULT_SIZE> struct FunList
{
    struct LI
    {
        T val;
        LI* next;
    };
    
    struct Allocator
    {
        struct Page
        {
            LI* lis;
            Page* next;
        };

        Allocator()
        {
            phead = NULL;
            plast = &phead;
            first = NULL;
        }

        ~Allocator()
        {
            Page* pg=phead;
            while(pg != NULL)
            {
                Page* tmp = pg;
                pg = pg->next;                
                delete [] tmp->lis;
                delete tmp;
            }
        }


        LI* get()
        {
            if (first == NULL)
                new_page();
            LI* res = first;
            first = first->next;
            return res;
        }

        void put(LI* l)
        {
            l->next = first;
            first = l;
        }

        void new_page()
        {
            Page* pg = new Page();
            *plast = pg;
            plast = &(pg->next);
            first = new LI[S];
            pg->lis = first;
            for (int i=0; i<S-1; ++i)
                first[i].next = &first[i+1];
            first[S-1].next = NULL;
        }

        Page* phead;
        Page** plast;
        LI* first;
    };

    FunList()
    {
        head = NULL;
        last = &head;
        sz = 0;
    }

    void push_front(const T& v)
    {
        head = newLI(v, head);
        ++sz;
    }

    void push_back(const T& v)
    {
        *last = newLI(v, NULL);
        last = &((*last)->next);
        ++sz;
    }

    void pop_front()
    {
        assert(head != NULL);
        LI* tmp = head;
        head = head->next;
        deleteLI(tmp);
        --sz;
    }

    void pop_back()
    {
        assert(head != NULL);
        LI* curr = head;
        LI** newlast = &head;
        while(curr->next != NULL)
        {
            newlast = &(curr->next);
            curr = curr->next;
        }
        deleteLI(curr);
        last = newlast;
        *last = NULL;
        --sz;
        
    }

    LI* begin() const
    {
        return head;
    }

    LI* end() const
    {
        return *last;
    }

    int size() const
    {
        return sz;
    }

    ~FunList()
    {
        LI* curr = head;
        while(curr != NULL)
        {
            LI* tmp = curr;
            curr = tmp->next;
            deleteLI(tmp);
        }
    }

protected:
    LI* newLI(const T& v, LI* nxt)
    {
        LI* l = allocator.get();
        l->val = v;
        l->next = nxt;
        return l;
    }

    void deleteLI(LI* l)
    {
        allocator.put(l);
    }

    Allocator allocator;
    int sz;
    LI* head;
    LI** last;
};
#endif

