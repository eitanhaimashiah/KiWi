

#include "ScanIndex.h"

namespace kiwi
{
	using kiwi::ThreadData::ScanData;

	ScanIndex<K>::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(ScanIndex<K*> *outerInstance)
	{
		this->outerInstance = outerInstance;
	}

	int ScanIndex<K>::ComparatorAnonymousInnerClass::compare(ScanData *o1, ScanData *o2)
	{
		return o2->version->get() - o1->version->get();
	}
}
