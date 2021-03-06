#ifndef IntegerGenerator_h
#define IntegerGenerator_h

#include <string>
#include "Generator.h"

using namespace std;

namespace util
{
	/* A generator that is capable of generating ints as well as strings */
	class IntegerGenerator : public Generator
	{
	public:
		int lastint = 0;

    protected:
        /**
         * Set the last value generated. IntegerGenerator subclasses must use this call
         * to properly set the last string value, or the lastString() and lastInt() calls won't work.
         */
        virtual void setLastInt(int last);

	public:
        /**
         * Return the next value as an int. When overriding this method, be sure to call setLastString() properly,
         * or the lastString() call won't work.
         */
        virtual int nextInt() = 0;

		virtual pair<int, int> nextInterval() = 0;

		/* Generate the next string in the distribution. */
		wstring nextString() override;

        /**
         * Return the previous string generated by the distribution; e.g., returned from the last nextString() call.
         * Calling lastString() should not advance the distribution or have any side effects. If nextString() has not yet
         * been called, lastString() should return something reasonable.
         */
		wstring lastString() override;

		/**
         * Return the previous int generated by the distribution. This call is unique to IntegerGenerator subclasses, and assumes
         * IntegerGenerator subclasses always return ints for nextInt() (e.g. not arbitrary strings).
         */
		virtual int lastInt();
		
        /**
         * Return the expected value (mean) of the values this generator will return.
         */
		virtual double mean() = 0;
	};

}

#endif /* IntegerGenerator */
