#ifndef Cell_h
#define Cell_h

#include <string>
#include <vector>

using namespace std;

namespace kiwi
{

	class Cell
	{
	public:
		static Cell const Empty;

	private:
		vector<char> const bytes;
		const int offset;
		const int length;

	public:
		Cell(vector<char> &bytes, int off, int len);

		virtual vector<char> getBytes();
		virtual int getOffset();
		virtual int getLength();
        
//		string toString() ;
//        bool equals(void *obj) ;
//        virtual int compareTo(Cell *c);
    
//    protected:
//		Cell *clone() ;
	};

}

#endif /* Cell_h */
