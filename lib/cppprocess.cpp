#include "cppprocess.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

typedef std::vector<std::vector<cv::Point>> Contours;

int const morph_elem = 0;
int const morph_size = 10;
int const morph_operator = 0;
int const max_operator = 4;
int const max_elem = 2;
int const max_kernel_size = 21;
cv::Mat element = cv::getStructuringElement(morph_elem, cv::Size(2*morph_size + 1, 2*morph_size + 1), cv::Point(morph_size, morph_size));
Contours findContoursOutput;
Contours filterContoursOutput;
Contours largestContours;
double minArea = 1.0;
double maxArea = 50000.0;
double minPerimeter = 50.0;
double minWidth = 20.0;
double minHeight = 40.0;
double maxHeight = 1500.0;
double maxWidth = 200.0;

void findContours(cv::Mat &input, Contours &contours);
void filterContours(const Contours &inputContours, Contours &output);
void overlayContours(cv::Mat &input, Contours &contours, cv::Scalar color = cv::Scalar(255, 0, 0));
void findLargestContoursByArea(Contours &contours, Contours &largestContours);

extern "C" {
void cppProcess(IplImage * img) {
  cv::Mat mat_img = cv::cvarrToMat(img);

  cv::Mat channels[3];
  cv::split(mat_img, channels);

  cv::Mat temp_img;
  // Step CV scaleAdd Green and Blue
  cv::scaleAdd(channels[0], -0.1, channels[1], temp_img);
  // Step CV scaleAdd output with Red
  cv::scaleAdd(channels[2], -0.7, temp_img, temp_img);

  cv::threshold(temp_img, temp_img, 75.0, 255.0, cv::THRESH_BINARY);

  cv::morphologyEx(temp_img, temp_img, cv::MORPH_CLOSE, element);

  findContours(temp_img, findContoursOutput);
  overlayContours(mat_img, findContoursOutput);

  filterContours(findContoursOutput, filterContoursOutput);
  // overlayContours(mat_img, filterContoursOutput, cv::Scalar(0, 255, 0));

  findLargestContoursByArea(filterContoursOutput, largestContours);
  if (!largestContours.empty()) {
      overlayContours(mat_img, filterContoursOutput, cv::Scalar(0, 255, 0));
  } else {
      std::cout << "Largest contour not found: " << std::endl;
  }
}
}


void findContours(cv::Mat &input, Contours &contours) {
    std::vector<cv::Vec4i> hierarchy;
    contours.clear();
    int mode = cv::RETR_LIST;
    int method = cv::CHAIN_APPROX_SIMPLE;
    cv::findContours(input, contours, hierarchy, mode, method);
}


void filterContours(const Contours &inputContours, Contours &output) {
    std::vector<cv::Point> hull;
    output.clear();

    for (const std::vector<cv::Point>& contour : inputContours) {
        cv::Rect bb = boundingRect(contour);
        std::cout << "Current bb: " << bb << std::endl;
        if (bb.width < minWidth) {
            std::cout << "Rejected at minWidth: " << minWidth << std::endl;
            continue;
        }
        if (bb.height < minHeight) {
            std::cout << "Rejected at minHeight: " << minHeight << std::endl;
            continue;
        }
        if (bb.height > maxHeight) {
            std::cout << "Rejected at maxHeight: " << maxHeight << std::endl;
            continue;
        }
        if (bb.width > maxWidth) {
            std::cout << "Rejected at maxWidth: " << maxWidth << std::endl;
            continue;
        }
        double area = cv::contourArea(contour);
        if (area < minArea) continue;
        if (area > maxArea) continue;
        if (arcLength(contour, true) < minPerimeter) continue;
        output.push_back(contour);
    }

    if (output.empty()) {
        std::cout << "No contours remaining: " << std::endl;
    } else {
        std::cout << "Contours found!" << std::endl;
    }
}


void overlayContours(cv::Mat &input, Contours &contours, cv::Scalar color) {
    cv::drawContours(input, contours, -1, color, 3);
}


void findLargestContoursByArea(Contours &contours, Contours &largestContours) {
    largestContours.clear();
    std::sort(contours.begin(), contours.end(), [](auto& x, auto& y)->bool{return cv::contourArea(x) > cv::contourArea(y);});

    Contours sorted_contours;

    for (unsigned int i = 0; i < contours.size() && i < 2; ++i) {
        largestContours.push_back(contours.at(i));
    }

    std::cout << "Largest areas: " << std::endl;
    for (std::vector<cv::Point> contour : largestContours) {
        std::cout << cv::contourArea(contour) << std::endl;
    }
}
