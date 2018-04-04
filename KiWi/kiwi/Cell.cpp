#include "Cell.h"

namespace kiwi
{
    std::vector<char> v = std::vector<char>(0);
    Cell const Cell::Empty (v, 0, 0);

	Cell::Cell(std::vector<char> &bytes, int off, int len) : bytes(bytes), offset(off), length(len)
	{
	}

	std::vector<char> Cell::getBytes()
	{
		return bytes;
	}

	int Cell::getOffset()
	{
		return offset;
	}

	int Cell::getLength()
	{
		return length;
	}

//	std::string Cell::toString()
//	{
//		if (length == 0)
//		{
//			return "Empty";
//		}
//
//		// TODO remove this method! works only for INTEGER!
//		int n = bytes[offset] << 24 | (bytes[offset + 1] & 0xFF) << 16 | (bytes[offset + 2] & 0xFF) << 8 | (bytes[offset + 3] & 0xFF);
//		return std::to_string(n) + "";
//	}
//
//	Cell *Cell::clone()
//	{
//		// allocate new byte array and copy data into it
//		std::vector<char> b(this->length);
//		System::arraycopy(bytes, offset, b, 0, length);
//
//		// return new Cell wrapping the cloned byte array
//		return new Cell(b, 0, length);
//	}
//
//	bool Cell::equals(void *obj)
//	{
//		return this->compareTo(static_cast<Cell*>(obj)) == 0;
//	}
//
//	int Cell::compareTo(Cell *c)
//	{
//		return ByteBuffer::wrap(bytes, offset, length)->compareTo(ByteBuffer::wrap(c->getBytes(), c->getOffset(), c->getLength()));
//	}
}
