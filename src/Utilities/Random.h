#pragma once

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

namespace Random {

	inline void initialize(){ srand(time(NULL)); }

	inline float getNormalized() { return (float)(rand() % RAND_MAX) / (float)RAND_MAX; }

	inline float getRanged(float min, float max) { return min + getNormalized() * (max - min); }

}
