#ifndef DebugStats_h
#define DebugStats_h

namespace kiwi
{
	class DebugStats
	{
    // TODO: I'm not sure why the convertor created the following class.
//	private:
//		class ThreadLocalAnonymousInnerClass : public ThreadLocal<Vars*>
//		{
//		public:
//			ThreadLocalAnonymousInnerClass();
//
//		protected:
////JAVA TO C++ CONVERTER WARNING: The following method was originally marked 'synchronized':
//			Vars *initialValue() override;
//		};
	public:
        long long itemCount = 0;
		long long jumpKeyCount = 0;
		long long jumpValCount = 0;
		long long chunksCount = 0;
		long long occupiedCells = 0;
		long long sortedCells = 0;
		long long duplicatesCount = 0;
		long long nulItemsCount = 0;
		long long removedItems = 0;
	};
}

#endif /* DebugStats_h */
