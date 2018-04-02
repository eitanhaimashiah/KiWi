#include <string>
#include <vector>

namespace kiwi
{



	/// <summary>
	/// Parameters of the Java version of the 
	/// Synchrobench benchmark.
	/// 
	/// @author Vincent Gramoli
	/// </summary>
	class Parameters
	{
	public:
		class KeyDistribution final
		{

//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			Uniform
//			{
//				public util.IntegerGenerator createGenerator(int min, int max)
//				{
//					return new UniformRandom(min,max);
//				}
//			},
//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			ShiftedUniform
//			{
//				public util.IntegerGenerator createGenerator(int min, int max)
//				{
//					return new ShiftedRandom(min, max);
//				}
//			},
//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			Zipfian
//			{
//				public util.IntegerGenerator createGenerator(int min, int max)
//				{
//					return new ZipfianRandomMiddle(min,max);
//				}
//			},
//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			ScrambledZipfian
//			{
//				public util.IntegerGenerator createGenerator(int min, int max)
//				{
//					throw new sun.reflect.generics.reflectiveObjects.NotImplementedException();
//				}
//			},
//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			SkewedLatest
//			{
//				public util.IntegerGenerator createGenerator(int min, int max)
//				{
//					throw new sun.reflect.generics.reflectiveObjects.NotImplementedException();
//				}
//			},
//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			Churn10Perc
//			{
//				public util.IntegerGenerator createGenerator(int min, int max)
//				{
//					return new ChurnPercRandom(min, max, 10);
//				}
//			},
//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			Churn5Perc
//			{
//				public util.IntegerGenerator createGenerator(int min, int max)
//				{
//					return new ChurnPercRandom(min, max, 5);
//				}
//			},
//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			Churn1Perc
//			{
//				public util.IntegerGenerator createGenerator(int min, int max)
//				{
//					return new ChurnPercRandom(min, max, 1);
//				}
//			};

private:
			static std::vector<KeyDistribution> valueList;

			class StaticConstructor
			{
			public:
				StaticConstructor();
			};

			static StaticConstructor staticConstructor;

public:
			enum class InnerEnum
			{
			};

			const InnerEnum innerEnumValue;
private:
			const std::wstring nameValue;
			const int ordinalValue;
			static int nextOrdinal;

			KeyDistribution(const std::wstring &name, InnerEnum innerEnum);

		public:
			virtual util::IntegerGenerator *createGenerator(int min, int max) = 0;

//JAVA TO C++ CONVERTER TODO TASK: Enum value-specific class bodies are not converted by Java to C++ Converter:
//			static class ChurnPercRandom extends util.IntegerGenerator
//			{
//				private java.util.Random rand;
//				int min;
//				int max;
//				int middle;
//				int churnSize;
//				public ChurnPercRandom(int min, int max, double churnPerc)
//				{
//					this.min = min;
//					this.max = max;
//					this.rand = new java.util.Random(System.nanoTime()^Thread.currentThread().getId());
//					this.middle = (max + min)/2;
//					this.churnSize = (int)((max-min)* churnPerc/100);
//				}
//				public int nextInt()
//				{
//					if(rand.nextInt(1000) > 100)
//					{
//						return Math.min(middle + rand.nextInt(churnSize),max);
//					}
//					else
//					{
//						return min + rand.nextInt(max - min);
//					}
//				}
//				public util.Pair<Integer, Integer> nextInterval()
//				{
//					throw new sun.reflect.generics.reflectiveObjects.NotImplementedException();
//				}
//				public double mean()
//				{
//					return 0;
//				}
//			}
//			static class UniformRandom extends util.IntegerGenerator
//			{
//				private int min;
//				private int max;
//				private java.util.Random rand;
//				public UniformRandom(int min, int max)
//				{
//					this.min = min;
//					this.max = max;
//					rand = new java.util.Random(System.nanoTime()^Thread.currentThread().getId());
//				}
//				public int nextInt()
//				{
//					return min + rand.nextInt(max - min);
//				}
//				public util.Pair<Integer, Integer> nextInterval()
//				{
//					int rangeSize = rand.nextInt(1+maxRangeSize-minRangeSize)+minRangeSize;
//					int min = nextInt() - rangeSize/2;
//					int minEdge = Parameters.range - rangeSize -1;
//					min = min < minEdge? min : minEdge;
//					int max = min + rangeSize;
//					return new util.Pair<>(min, max);
//				}
//				public double mean()
//				{
//					return (double)(max+min)/2;
//				}
//			}
//			static class ShiftedRandom extends util.IntegerGenerator
//			{
//				private final int shift = util.CombinedGenerator.getShift();
//				private UniformRandom uniformRandom;
//				public ShiftedRandom(int min, int max)
//				{
//					uniformRandom = new UniformRandom(min,max);
//				}
//				private int shiftValue(int value)
//				{
//					return value << shift;
//				}
//				public int nextInt()
//				{
//					return shiftValue(uniformRandom.nextInt());
//				}
//				public util.Pair<Integer, Integer> nextInterval()
//				{
//					util.Pair<Integer,Integer> pair = uniformRandom.nextInterval();
//					int left = shiftValue(pair.getLeft());
//					int right = shiftValue(pair.getRight());
//					return new util.Pair<>(left,right);
//				}
//				public double mean()
//				{
//					return ((double)(shiftValue(uniformRandom.min) + shiftValue(uniformRandom.max)))/2;
//				}
//			}
//			static class ZipfianRandomMiddle extends util.IntegerGenerator
//			{
//				private util.ZipfianGenerator zGen;
//				private int min;
//				private int max;
//				private int middle;
//				private java.util.Random sideRand;
//				public ZipfianRandomMiddle(int min, int max)
//				{
//					this.min = min;
//					this.max = max;
//					this.middle = (min + max)/2;
//					this.sideRand = new java.util.Random(System.nanoTime()^Thread.currentThread().getId());
//					zGen = new util.ZipfianGenerator(middle, max);
//				}
//				public int nextInt()
//				{
//					int diff = zGen.nextInt() -middle;
//					if(sideRand.nextBoolean())
//					{
//						return middle -diff;
//					}
//					else
//					{
//						return middle + diff;
//					}
//				}
//				public util.Pair<Integer, Integer> nextInterval()
//				{
//					throw new sun.reflect.generics.reflectiveObjects.NotImplementedException();
//				}
//				public double mean()
//				{
//					throw new sun.reflect.generics.reflectiveObjects.NotImplementedException();
//				}
//			}

public:
			bool operator == (const KeyDistribution &other);

			bool operator != (const KeyDistribution &other);

			static std::vector<KeyDistribution> values();

			int ordinal();

			std::wstring toString();

			static KeyDistribution valueOf(const std::wstring &name);
		};
		enum class FillType
		{
			java.util.Random,
			Ordered,
			DropHalf,
			Drop90
		};

	public:
		static KeyDistribution distribution;
		static int numThreads;
		static int numMilliseconds;
		static int numWrites;
		static int numWriteAlls;
		static int numSnapshots;
		static int range;
		static int size;
		static int warmUp;
		static int iterations;

		static bool detailedStats;

		static std::wstring benchClassName;
		static bool rangeQueries;
		static int minRangeSize;
		static int maxRangeSize;

		static bool countKeysInRange;
		static double rebalanceProbPerc;
		static double sortedRebalanceRatio;

		static FillType fillType;

		static AtomicInteger *compactionsNum;
		static AtomicInteger *engagedChunks;
		static bool countCompactions;

	};

}