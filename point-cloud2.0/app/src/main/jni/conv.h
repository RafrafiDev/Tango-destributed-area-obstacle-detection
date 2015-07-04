#include <cstddef>

typedef char* schar;
float bytesToFloat(char b0, char b1, char b2, char b3);
schar floatToBytesArray(float* from,size_t size);
float* bytesToFloatArray(schar from,size_t size);
