struct info {
	uint32_t seed; /* start parent */
	uint32_t mask; /* which bits to mutate */
};

#include <map>

extern map<string, struct info> lookup;
