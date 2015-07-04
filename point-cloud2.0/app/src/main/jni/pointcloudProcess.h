#ifndef POINT_CLOUD_JNI_EXAMPLE_POINTCLOUDPROCESS_H_
#define POINT_CLOUD_JNI_EXAMPLE_POINTCLOUDPROCESS_H_


class PointcloudProcess {
 public:
  static void zClippingColor(float* depthbuffer,int depth_size,float* colorbuffer,int color_size,float far, float left, float right,float top,float bottom);
};

#endif  // POINT_CLOUD_JNI_EXAMPLE_POINTCLOUD_H_
