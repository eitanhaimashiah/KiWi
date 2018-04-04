

#include "ZipfianGenerator.h"
#include "Utils.h"
#include "ScrambledZipfianGenerator.h"

namespace util
{
	ZipfianGenerator::ZipfianGenerator(long long _items) : ZipfianGenerator(0,_items - 1)
	{
	}

	ZipfianGenerator::ZipfianGenerator(long long _min, long long _max) : ZipfianGenerator(_min,_max,ZIPFIAN_CONSTANT)
	{
	}

	ZipfianGenerator::ZipfianGenerator(long long _items, double _zipfianconstant) : ZipfianGenerator(0,_items - 1,_zipfianconstant)
	{
	}

	ZipfianGenerator::ZipfianGenerator(long long min, long long max, double _zipfianconstant) : ZipfianGenerator(min,max,_zipfianconstant,zetastatic(max - min + 1,_zipfianconstant))
	{
	}

	ZipfianGenerator::ZipfianGenerator(long long min, long long max, double _zipfianconstant, double _zetan)
	{

		items = max - min + 1;
		base = min;
		zipfianconstant = _zipfianconstant;

		theta = zipfianconstant;

		zeta2theta = zeta(2,theta);


		alpha = 1.0 / (1.0 - theta);
		//zetan=zeta(items,theta);
		zetan = _zetan;
		countforzeta = items;
		eta = (1 - pow(2.0 / items,1 - theta)) / (1 - zeta2theta / zetan);

		//System.out.println("XXXX 3 XXXX");
		nextInt();
		//System.out.println("XXXX 4 XXXX");
	}

	double ZipfianGenerator::zeta(long long n, double theta)
	{
		countforzeta = n;
		return zetastatic(n,theta);
	}

	double ZipfianGenerator::zetastatic(long long n, double theta)
	{
		return zetastatic(0,n,theta,0);
	}

	double ZipfianGenerator::zeta(long long st, long long n, double theta, double initialsum)
	{
		countforzeta = n;
		return zetastatic(st,n,theta,initialsum);
	}

	double ZipfianGenerator::zetastatic(long long st, long long n, double theta, double initialsum)
	{
		double sum = initialsum;
		for (long long i = st; i < n; i++)
		{

			sum += 1 / (pow(i + 1,theta));
		}

		//System.out.println("countforzeta="+countforzeta);

		return sum;
	}

	int ZipfianGenerator::nextInt(int itemcount)
	{
		return static_cast<int>(nextLong(itemcount));
	}

	long long ZipfianGenerator::nextLong(long long itemcount)
	{
		//from "Quickly Generating Billion-Record Synthetic Databases", Jim Gray et al, SIGMOD 1994

		if (itemcount != countforzeta)
		{

			//have to recompute zetan and eta, since they depend on itemcount
		//	synchronized(this)
			{
				if (itemcount > countforzeta)
				{
					//System.err.println("WARNING: Incrementally recomputing Zipfian distribtion. (itemcount="+itemcount+" countforzeta="+countforzeta+")");

					//we have added more items. can compute zetan incrementally, which is cheaper
					zetan = zeta(countforzeta,itemcount,theta,zetan);
					eta = (1 - pow(2.0 / items,1 - theta)) / (1 - zeta2theta / zetan);
				}
				else if ((itemcount < countforzeta) && (allowitemcountdecrease))
				{
					//have to start over with zetan
					//note : for large itemsets, this is very slow. so don't do it!

					//TODO: can also have a negative incremental computation, e.g. if you decrease the number of items, then just subtract
					//the zeta sequence terms for the items that went away. This would be faster than recomputing from scratch when the number of items
					//decreases

                    cout << ("WARNING: Recomputing Zipfian distribtion. This is slow and should be avoided. (itemcount=" + to_string(itemcount) + " countforzeta=" + to_string(countforzeta) + ")\n");

					zetan = zeta(itemcount,theta);
					eta = (1 - pow(2.0 / items,1 - theta)) / (1 - zeta2theta / zetan);
				}
			}
		}

		double u = Utils::nextDouble();
		double uz = u*zetan;

		if (uz < 1.0)
		{
			return 0;
		}

		if (uz < 1.0 + pow(0.5,theta))
		{
			return 1;
		}

		long long ret = base + static_cast<long long>((itemcount) * pow(eta*u - eta + 1, alpha));
		setLastInt(static_cast<int>(ret));
		return ret;
	}

	int ZipfianGenerator::nextInt()
	{
		return static_cast<int>(nextLong(items));
	}

    pair<int, int> ZipfianGenerator::nextInterval()
	{
		throw NotImplementedException();
	}

	long long ZipfianGenerator::nextLong()
	{
		return nextLong(items);
	}


	double ZipfianGenerator::mean()
	{
		throw UnsupportedOperationException("@todo implement ZipfianGenerator.mean()");
	}
}
