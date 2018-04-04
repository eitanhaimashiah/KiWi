#ifndef PutData_h
#define PutData_h

/* Thread data class for Put operations */
class PutData
{
public:
    const int orderIndex;
    
    PutData(int orderIndex) : orderIndex(orderIndex) {}
};

#endif /* PutData_h */
