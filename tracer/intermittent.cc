#include "intermittent.h"

#include <stdlib.h>

void intermittent_t::fail() {
	fail_cycle = intermittent_min + (rand() % static_cast<int>(
        intermittent_max - intermittent_min + 1));	
}