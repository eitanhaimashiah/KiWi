#include "Chunk.h"

namespace kiwi
{
	using kiwi::ThreadData::PutData;
	using sun::misc::Unsafe;

	Chunk<K, V>::Statistics::Statistics(Chunk<K, V> *outerInstance) : outerInstance(outerInstance)
	{
	}

	int Chunk<K, V>::Statistics::getMaxItems()
	{
		return outerInstance->MAX_ITEMS;
	}

	int Chunk<K, V>::Statistics::getFilledCount()
	{
		return outerInstance->orderIndex->get() / outerInstance->ORDER_SIZE;
	}

	int Chunk<K, V>::Statistics::getCompactedCount()
	{
		return getFilledCount() - getDuplicatesCount();
	}

	void Chunk<K, V>::Statistics::incrementDuplicates()
	{
		dupsCount->addAndGet(1);
	}

	int Chunk<K, V>::Statistics::getDuplicatesCount()
	{
		return dupsCount->get();
	}

	Chunk<K, V>::StaticConstructor::StaticConstructor()
	{
	try
	{
		Constructor<Unsafe*> *unsafeConstructor = Unsafe::typeid->getDeclaredConstructor();
		unsafeConstructor->setAccessible(true);
		unsafe = unsafeConstructor->newInstance();
	}
	catch (const std::exception &ex)
	{
//JAVA TO C++ CONVERTER TODO TASK: The std::exception constructor has no parameters:
//ORIGINAL LINE: throw new RuntimeException(ex);
		throw std::exception();
	}
	}

	Chunk<K, V>::ItemsIterator::ItemsIterator(Chunk<K, V> *outerInstance) : outerInstance(outerInstance)
	{
		current = HEAD_NODE;
		iterVersions = new VersionsIterator(this);
	}

	bool Chunk<K, V>::ItemsIterator::hasNext()
	{
			return outerInstance->get(current, OFFSET_NEXT) != NONE;
	}

	void Chunk<K, V>::ItemsIterator::next()
	{
		current = outerInstance->get(current, OFFSET_NEXT);
		iterVersions->justStarted = true;
	}

	K *Chunk<K, V>::ItemsIterator::getKey()
	{
		return outerInstance->readKey(current);
	}

	V *Chunk<K, V>::ItemsIterator::getValue()
	{
		return outerInstance->getData(current);
	}

	int Chunk<K, V>::ItemsIterator::getVersion()
	{
		return outerInstance->getVersion(current);
	}

	ItemsIterator *Chunk<K, V>::ItemsIterator::cloneIterator()
	{
		ItemsIterator *newIterator = new ItemsIterator(outerInstance);
		newIterator->current = this->current;
		return newIterator;
	}

	Chunk::ItemsIterator::VersionsIterator *Chunk<K, V>::ItemsIterator::versionsIterator()
	{
		return iterVersions;
	}

	Chunk<K, V>::ItemsIterator::VersionsIterator::VersionsIterator(Chunk::ItemsIterator *outerInstance) : outerInstance(outerInstance)
	{
	}

	V *Chunk<K, V>::ItemsIterator::VersionsIterator::getValue()
	{
		return outerInstance->outerInstance->getData(outerInstance->current);
	}

	int Chunk<K, V>::ItemsIterator::VersionsIterator::getVersion()
	{
		return outerInstance->outerInstance->getVersion(outerInstance->current);
	}

	bool Chunk<K, V>::ItemsIterator::VersionsIterator::hasNext()
	{
		if (justStarted)
		{
			return true;
		}
		int next = outerInstance->outerInstance->get(outerInstance->current,OFFSET_NEXT);

		if (next == Chunk::NONE)
		{
			return false;
		}
		return outerInstance->outerInstance->readKey(outerInstance->current)(outerInstance->outerInstance->readKey(next)) == 0;
	}

	void Chunk<K, V>::ItemsIterator::VersionsIterator::next()
	{
		if (justStarted)
		{
			justStarted = false;
			return;
		}
		else
		{
			outerInstance->current = outerInstance->outerInstance->get(outerInstance->current,OFFSET_NEXT);
		}
	}
}
