// ledmatrix.cpp
#include "ledmatrix.h"

// Define your frame data
const uint32_t happy[] = {
  0x19819,
  0x80000001,
  0x81f8000
};

const uint32_t heart[] = {
  0x3184a444,
  0x44042081,
  0x100a0040
};

const uint32_t one[] = {
		0x4004004,
		0x400400,
		0x40040040,
		66
	};

const uint32_t testnum[] = {
	
		0xb6e92ab6,
		0xaa2ab6e0,
		0x0,
		66

};

// Return frame pointer by name
const uint32_t* getArtFrame(const char* name) {
  if (strcmp(name, "happy") == 0) return happy;
  if (strcmp(name, "heart") == 0) return heart;
  if (strcmp(name, "one") == 0) return one;
  if (strcmp(name, "testnum") == 0) return testnum;
  return nullptr;
}
