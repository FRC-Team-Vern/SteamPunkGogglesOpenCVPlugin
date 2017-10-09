#include "cppprocess.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>


// Contours typedef for simpler coding of parameters
typedef std::vector<std::vector<cv::Point>> Contours;

// Declare Global contants
const cv::Scalar RED(255, 0, 0);
const cv::Scalar GREEN(0, 255, 0);
const cv::Scalar BLUE(0, 0, 255);
const int morph_elem = 0;
const int morph_size = 10;
const int morph_operator = 0;
const int max_operator = 4;
const int max_elem = 2;
const int max_kernel_size = 21;
const cv::Mat element = cv::getStructuringElement(morph_elem, cv::Size(2*morph_size + 1, 2*morph_size + 1), cv::Point(morph_size, morph_size));
const int mode = cv::RETR_LIST;
const int method = cv::CHAIN_APPROX_SIMPLE;
const double minArea = 1.0;
const double maxArea = 50000.0;
const double minPerimeter = 50.0;
const double minWidth = 20.0;
const double minHeight = 40.0;
const double maxHeight = 1500.0;
const double maxWidth = 200.0;
const int SHIFTEDCENTER = 400;

// Declare Global variables
Contours findContoursOutput;
Contours filterContoursOutput;
Contours largestContours;
std::pair<cv::Rect, cv::Rect> TLR;
cv::Point center;
int radius;
int targetX;

void findContours(cv::Mat &input);
void filterContours(const Contours &inputContours, Contours &output);
void overlayContours(cv::Mat &input, Contours &contours, cv::Scalar color = RED);
void findLargestContoursByArea(Contours &contours, Contours &largestContours);
void findFinalTarget(const Contours& contours);
void drawFinalTarget(cv::Mat& src);
void calcXShift();


extern "C" {
int cppProcess(IplImage * img) {
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

  findContours(temp_img);
  overlayContours(mat_img, findContoursOutput);

  filterContours(findContoursOutput, filterContoursOutput);

  findLargestContoursByArea(filterContoursOutput, largestContours);
  if (!largestContours.empty()) {
      overlayContours(mat_img, filterContoursOutput, GREEN);
  } else {
      std::cout << "Largest contour not found: " << std::endl;
  }

  findFinalTarget(largestContours);

  drawFinalTarget(mat_img);

  return targetX;
}
}


void findContours(cv::Mat &input) {
    std::vector<cv::Vec4i> hierarchy;
    findContoursOutput.clear();
    cv::findContours(input, findContoursOutput, hierarchy, mode, method);
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

    // This sort uses a C++11-style lamda function with auto type arguments
    std::sort(contours.begin(), contours.end(),
              [](auto& x, auto& y)->bool{return cv::contourArea(x) > cv::contourArea(y);});

    Contours sorted_contours;

    // This for loop pulls the two largest contours by area off due to their having been sorted
    for (unsigned int i = 0; i < contours.size() && i < 2; ++i) {
        largestContours.push_back(contours.at(i));
    }

    // Report largest areas of contours
    std::cout << "Largest areas: " << std::endl;
    for (std::vector<cv::Point> contour : largestContours) {
        std::cout << cv::contourArea(contour) << std::endl;
    }
}


/**
 * @brief findFinalTarget Uses two largest contours to determine Rect boundaries
 * @param contours This value is assumed to only have the top two largest contours
 */
void findFinalTarget(const Contours& contours) {

    // Two zero-sized Rects mean that there are no contours found
    TLR = std::make_pair(cv::Rect(0,0,0,0),cv::Rect(0,0,0,0));

    // Iterator over contours and compare bounding Rects
    for (const std::vector<cv::Point>& contour : contours) {

        cv::Rect bb = cv::boundingRect(cv::Mat(contour));

        // If the first value has already been written to then write to second
        if(TLR.first.tl().x > 0) {
                TLR.second = bb;
        } else {
                TLR.first = bb;
        }
    }

    // Gets skipped if there are no contours
    if(contours.size() > 1) {
        std::cout<<"first Rect: "<<TLR.first<<std::endl;
        std::cout<<"second Rect: "<<TLR.second<<std::endl;

        // Swap if Rects are out of order from left to right
        if (TLR.first.tl().x > TLR.second.tl().x) {
            TLR = std::make_pair(TLR.second, TLR.first);
        }
        cv::Point Tl = TLR.first.tl();
        cv::Point Br = TLR.second.br();
        center = (Tl + Br)*.5;
        radius = ((Br.x - Tl.x)*.5);
        calcXShift();

    //#if DEBUG
        std::cout << "Tl: " <<Tl << std::endl
            << "Br: " <<Br << std::endl
            << "center: " << center << std::endl
            << "radius: " << radius << std::endl
            << "Target X: " << targetX << std::endl;
    //#endif
    } else {
        radius = 0;
        targetX = -999;
    }
}


/**
 * @brief drawFinalTarget
 * @param src
 */
void drawFinalTarget(cv::Mat& src) {
    // Only print target if there is one.
    if (radius != 0) {
        cv::rectangle(src, TLR.first.tl(), TLR.first.br(), BLUE, 2);
        cv::rectangle(src, TLR.second.tl(), TLR.second.br(), BLUE, 2);
        cv::circle(src, center, radius, RED, 2);
        cv::circle(src, center, 2, RED, 2);
    }
}


void calcXShift() {
    targetX = center.x - SHIFTEDCENTER;
}
