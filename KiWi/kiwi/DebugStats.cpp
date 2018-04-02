

#include "DebugStats.h"

namespace kiwi
{

	DebugStats::ThreadLocalAnonymousInnerClass::ThreadLocalAnonymousInnerClass()
	{
	}

//JAVA TO C++ CONVERTER WARNING: The following method was originally marked 'synchronized':
	Vars *DebugStats::ThreadLocalAnonymousInnerClass::initialValue()
	{
		return new Vars();
	}
}
