#ifndef TIMER_HPP
#define TIMER_HPP

#include <time.h>

template <typename _>
class Timer_base {
public:
	
	void reset() { start = clock(); }
	
	Timer_base() { reset(); }
	
	double time() { 
		
		cur = clock();
		return ((double)cur - (double)start) / CPS;
		
	}
	
private:
	
	clock_t start;
	clock_t cur;
	
	const static double CPS;
	
};

template <typename _>
const double Timer_base<_>::CPS = (double)CLOCKS_PER_SEC;

using Timer = Timer_base<void>;

#endif