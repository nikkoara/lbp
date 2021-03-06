#ifndef LBP_CSSILTP2_HPP
#define LBP_CSSILTP2_HPP

#include <lbp/defs.hpp>
#include <lbp/utils.hpp>
#include <lbp/detail/neighborhoods.hpp>
#include <lbp/detail/sampling.hpp>

#include <opencv2/core.hpp>

#include <boost/hana/integral_constant.hpp>
#include <boost/hana/functional/demux.hpp>

#include <boost/integer.hpp>

//
// @article{wu2014real,
//   title={Real-time background subtraction-based video surveillance of people by integrating local texture patterns},
//   author={Wu, Hefeng and Liu, Ning and Luo, Xiaonan and Su, Jiawei and Chen, Liangshi},
//   journal={Signal, Image and Video Processing},
//   volume={8},
//   number={4},
//   pages={665--676},
//   year={2014},
//   publisher={Springer}
// }
//

namespace lbp {
namespace cssiltp2_detail {

template< typename T >
auto cssiltp2 = [](auto N, auto S) {
    return [=](const cv::Mat& src, size_t i, size_t j, const T& tau) {
        using namespace cv;
        using namespace hana::literals;

        return hana::fold_left (
            N, 0, [&, shift = 0](auto accum, auto x) mutable {
                const auto a = S (src, i + x [0_c], j + x [1_c]);
                const auto b = S (src, i - x [0_c], j - x [1_c]);

                const auto lhs = saturate_cast< T > ((1. - tau) * a);
                const auto rhs = saturate_cast< T > ((1. + tau) * a);

                return accum | (
                    (b > rhs ? 1 : (b < lhs ? 2 : 0)) << (shift++ * 2));
            });
    };
};

} // namespace cssiltp2_detail

template< typename T, size_t R, size_t P >
auto cssiltp2 = [](const T& tau = T { }) {
    return [=](const cv::Mat& src) {
        LBP_STATIC_ASSERT_MSG (0 == (P % 2), "odd-sized neighborhood");

        using value_type = typename boost::uint_t< P >::least;

        cv::Mat dst (src.size (), opencv_type< value_type >, cv::Scalar (0));

        auto op = cssiltp2_detail::cssiltp2< T > (
            detail::semicircular_neighborhood< R, P >,
            detail::nearest_sampler< T >);

#pragma omp parallel for
        for (size_t i = R; i < src.rows - R; ++i) {
            for (size_t j = R; j < src.cols - R; ++j) {
                dst.at< value_type > (i, j) = op (src, i, j, tau);
            }
        }

        return dst;
    };
};

} // namespace lbp

#endif // LBP_CSSILTP2_HPP
