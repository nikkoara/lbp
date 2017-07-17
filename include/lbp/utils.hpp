#ifndef LBP_COMMON_HPP
#define LBP_COMMON_HPP

#include <lbp/defs.hpp>
#include <opencv2/imgproc.hpp>

#include <chrono>

namespace lbp {

cv::Mat
convert (const cv::Mat&, int, double a = 1, double b = 0);

cv::Mat
float_from (const cv::Mat&, double scale = 1. / 255, double offset = 0.);

cv::Mat
mono_from (const cv::Mat&, double scale = 255., double offset = 0.);

cv::Mat
mono_integral_from (const cv::Mat&, double scale = 1., double offset = 0.);

cv::Mat
median_blur (const cv::Mat&, int size = 3);

cv::Mat
multiply (const cv::Mat&, const cv::Mat&);

cv::Mat
convert_color (const cv::Mat&, int);

int
gray_from (int r, int g, int b);

int
gray_from (const cv::Vec3b& arg);

cv::Mat
gray_from (const cv::Mat&);

cv::Mat
threshold (const cv::Mat&, double threshold = 1.0,
           double maxval = 255.0, int type = CV_THRESH_BINARY);

cv::Mat
power_of (const cv::Mat&, double);

cv::Mat
absdiff (const cv::Mat&, const cv::Mat&);

cv::Mat
square_of (const cv::Mat&);

cv::Mat
bitwise_and (const cv::Mat&, const cv::Mat&);

cv::Mat
bitwise_and (const cv::Mat&, const cv::Mat&, const cv::Mat&);

cv::Mat
bitwise_not (cv::Mat);

cv::Mat
bitwise_not (const cv::Mat&, const cv::Mat&);

cv::Mat
mask (const cv::Mat&, const cv::Mat&);

cv::Mat
flip (const cv::Mat&, int how = 0);

int
chebyshev (const cv::Vec3b&, const cv::Vec3b&);

cv::Mat
scale_frame (cv::Mat&, size_t to = 512);

struct frame_delay {
    explicit frame_delay (size_t value = 40 /* milliseconds */);
    bool wait_for_key (int key) const;

private:
    int value_;
    std::chrono::high_resolution_clock::time_point begin_;
};

template< size_t > constexpr int opencv_type;

#define OPENCV_TYPEDEF(n, t) template< > constexpr int opencv_type< n > = t

OPENCV_TYPEDEF ( 8, CV_8U);
OPENCV_TYPEDEF (16, CV_16U);
OPENCV_TYPEDEF (32, CV_32S);

#undef OPENCV_TYPE_DEF

} // namespace lbp

#endif // LBP_COMMON_HPP
