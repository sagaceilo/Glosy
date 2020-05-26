#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <algorithm>
#include <intrin0.h>


static constexpr unsigned arr[] =
{
	177,
	193,
	185,
	157,
	200,
	200,
	354,
	341,
	186,
	177,
	166,
	184,
	156,
	204,
	200,
	353,
	282,
	190,
	177,
	167,
	185,
	155,
	202,
	198,
	353,
	281,
	185,
	176,
	359,
	154,
	203,
	191,
	351,
	530,
	267,
	192,
	401,
	749,
	545,
	 69
};

static constexpr int numLoc = sizeof( arr ) / sizeof( arr[0] );
static constexpr int numThr = 12;

static const char* names[numLoc] =
{
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"8",
	"9",
	"10",
	"11",
	"12",
	"13",
	"14",
	"15",
	"16",
	"17",
	"18",
	"19",
	"20",
	"21",
	"22",
	"23",
	"24",
	"25",
	"26",
	"27",
	"28",
	"29",
	"30/31",
	"32",
	"33",
	"34",
	"35",
	"36",
	"U1",
	"U2",
	"U3",
	"U4",
	"U5",
	"PG1",
};

static constexpr bool excl[numLoc] =
{
	1,	// 1
	0,	// 2
	1,	// 3
	0,	// 4
	1,	// 5
	1,	// 6
	1,	// 8
	0,	// 9
	0,	// 10
	1,	// 11
	1,	// 12
	0,	// 13
	0,	// 14
	0,	// 15
	1,	// 16
	1,	// 17
	0,	// 18
	0,	// 19
	1,	// 20
	1,	// 21
	1,	// 22
	1,	// 23
	1,	// 24
	1,	// 25
	1,	// 26
	0,	// 27
	0,	// 28
	1,	// 29
	1,	// 30/31
	1,	// 32
	0,	// 33
	1,	// 34
	1,	// 35
	0,	// 36
	0,	// U1
	0,	// U2
	1,	// U3
	1,	// U4
	1,	// U5
	0	// PG1
};

std::mutex printLock;
std::atomic<int> count = 0;

unsigned histogram[numLoc];

// **************************************************************************************************** //

int main()
{
	static const int target = 5525;
	static const int margin =    1;

	std::thread threads[numThr];

	constexpr unsigned long long mask	= (1ull << numLoc) - 1;
	constexpr unsigned long long bin	= mask / numThr;

	unsigned long long from = 0;

	for( int t = 0; t < numThr; ++t )
	{
		unsigned long long to = from + bin;

		threads[t] = std::thread( [=]()
								  {
									  unsigned long long exlMask = 0;
									  {
										  unsigned long long f = 1;

										  for( unsigned m = 0; m < numLoc; ++m, f <<= 1 )
										  {
											  exlMask |= excl[m] ? 0 : f;
										  }
									  }

									  for( unsigned long long i = from; i < to; ++i )
									  {
										  // Skip apartments that we know didn't voted
										  if( i & exlMask )
											  continue;

										  int sum = 0;
										  {
											  unsigned long long k = i;

											  while( k )
											  {
												  unsigned long index = 0;
												  ::_BitScanForward64( &index, k );
												  k &= k - 1;

												  sum += arr[index];
											  }
										  }

										  if( sum >= target - margin && sum <= target + margin )
										  {
											  std::lock_guard<std::mutex> lock( printLock );

											  std::cout << "Found [" << ++count <<"] ( " << sum * 0.01f <<  "% : +/-" << std::abs( sum - target ) * 0.01f << ")!!\nLokale: ";

											  unsigned long long f = 1;

											  for( unsigned m = 0; m < numLoc; ++m, f <<= 1 )
											  {
												  if( 0 != (i & f) )
												  {
													  std::cout << names[m] << ", ";

													  ++histogram[m];
												  }
											  }

											  std::cout << "\n\n";
										  }
									  }

								  } );

		from += bin;
	}

	for( int t = 0; t < numThr; ++t )
	{
		threads[t].join();
	}

	std::pair<int, int> idx[numLoc];
	int numUsed = 0;

	for( int i = 0; i < numLoc; ++i )
	{
		if( histogram[i] && excl[i] )
		{
			idx[numUsed++] ={ i, histogram[i] };
		}
	}

	std::sort( idx, idx + numUsed, []( auto a, auto b ) { return a.second > b.second; } );

	std::cout << "Histogram: \n";

	for( int i = 0; i < numUsed; ++i )
	{
		std::cout << "* Lokal: " << names[idx[i].first] << " - " << ( 100 * idx[i].second / count) << "%\n";
	}

	std::cin.get();
}

// **************************************************************************************************** //
