#include <conv.h>

float bytesToFloat(char b0, char b1, char b2, char b3)
{
    float res;
    *(schar(&res) + 3) = b0;
    *(schar(&res) + 2) = b1;
    *(schar(&res) + 1) = b2;
    *(schar(&res) + 0) = b3;
    return res;
}

schar floatToBytesArray(float* from,size_t size)
{
	schar to=new char[size*sizeof(float)+6];
	int j=0;
	to[0]='(';
    	to[1]='.';
    	to[2]='(';
        	to[3]='.';
        	to[4]='(';
            	to[5]='.';

	for(int i=6;i<size*sizeof(float)+6;i+=4) {
		to[i  ]=*(schar(&from[j]) + 3);
		to[i+1]=*(schar(&from[j]) + 2);
		to[i+2]=*(schar(&from[j]) + 1);
		to[i+3]=*(schar(&from[j]) + 0);
		j++;
	}

	return to;
}

float* bytesToFloatArray(schar from,size_t size)
{
	float* to=new float[size/sizeof(float)];
	int j=0;
	for(int k=0;k<size;k+=4) {
	 	to[j]=bytesToFloat(from[k], from[k+1], from[k+2], from[k+3]);
		j++;	
	}
	return to;
}

