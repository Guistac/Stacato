#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

namespace Random {

	bool b_seedInitialized = false;

	float getNormalized() {
		if (!b_seedInitialized) {
			srand(time(NULL));
			b_seedInitialized = true;
		}
		return (float)(rand() % RAND_MAX) / (float)RAND_MAX;
	}

	float getRanged(float min, float max) {
		return min + getNormalized() * (max - min);
	}

}