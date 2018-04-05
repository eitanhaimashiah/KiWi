#ifndef MarkableReference_h
#define MarkableReference_h

#include <stdio.h>

template<typename T>
class MarkableReference
{
private:
    uintptr_t val;
    static const uintptr_t mask = 1;
public:
    // TODO: Check if i can get rid of the raw pointer
    MarkableReference(T* ref = nullptr, bool mark = false)
    {
        val = ((uintptr_t)ref & ~mask) | (mark ? 1 : 0);
    }
    
    T* getReference()
    {
        return (T*)(val & ~mask);
    }
    
    bool isMarked()
    {
        return (val & mask);
    }
};

#endif /* MarkableReference_h */
