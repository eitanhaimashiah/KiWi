#ifndef Cell_h
#define Cell_h

#include <string>
#include <vector>

namespace kiwi
{

	class Cell
	{
	public:
		static Cell const Empty;

	private:
		std::vector<char> const bytes;
		const int offset;
		const int length;

	public:
		Cell(std::vector<char> &bytes, int off, int len);

		virtual std::vector<char> getBytes();
		virtual int getOffset();
		virtual int getLength();
        
//		std::string toString() ;
//        bool equals(void *obj) ;
//        virtual int compareTo(Cell *c);
    
//    protected:
//		Cell *clone() ;
	};

}

#endif /* Cell_h */
