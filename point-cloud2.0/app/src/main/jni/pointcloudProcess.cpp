#include "pointcloudProcess.h"
#include "tango-gl/util.h"





void PointcloudProcess::zClippingColor(float* depthbuffer,int depth_size,float* colorbuffer,int color_size,float far, float left, float right,float top,float bottom){
float xmax=0.0f,ymax=0.0f,zmax=0.0f;
float xmin=0.0f,ymin=0.0f,zmin=0.0f;

  for(int k=0;k<3 * color_size;k+=3){

 /*   //x max =+3
        if (xmax<depthbuffer[k])xmax=depthbuffer[k];
    //x min =-3
        if (xmin>depthbuffer[k])xmin=depthbuffer[k];

    //y max
        if (ymax<depthbuffer[k+1])ymax=depthbuffer[k];

    //y min
        if (ymin>depthbuffer[k+1])ymin=depthbuffer[k];


    //z max
        if (zmax<depthbuffer[k+2])zmax=depthbuffer[k];

    //z min
        if (zmin>depthbuffer[k+2])zmin=depthbuffer[k];

*/


    if(depthbuffer[k+2]<far && depthbuffer[k] > left && depthbuffer[k] < right){
      colorbuffer[k]=1;

      colorbuffer[k+1]=0;

      colorbuffer[k+2]=0;

    }
    else{
          colorbuffer[k]=0;

          colorbuffer[k+1]=1;

          colorbuffer[k+2]=0;

    }
    /*
if(depthbuffer[k]>-2.3 && depthbuffer[k]<2.3 && depthbuffer[k+1]>-0.3 && depthbuffer[k+1]<0.3){
      colorbuffer[k]=0;

      colorbuffer[k+1]=0;

      colorbuffer[k+2]=1;

    }
*/

  }

  //__android_log_print(ANDROID_LOG_INFO, "Range values", "zmax %f zmin %f xman  %f xmin %f ymax %f ymin %f", zmax, zmin, xmax, xmin, ymax, ymin);
}