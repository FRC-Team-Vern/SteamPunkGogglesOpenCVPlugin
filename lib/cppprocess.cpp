#include "cppprocess.h"
#include <opencv2/core/core.hpp>

extern "C" {
IplImage * cppProcess(IplImage * img) {
  cv::Mat mat_img = cv::cvarrToMat(img);

  cv::Mat color_planes[3];
  cv::split(mat_img, color_planes);

  return new IplImage(color_planes[0]);
}
}
