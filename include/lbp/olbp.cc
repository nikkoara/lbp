// -*- mode: c++; -*-

#include <lbp/olbp.hpp>
#include <lbp/utils.hpp>

#include <functional>

namespace lbp {
namespace olbp_detail {

inline size_t
count_ones (size_t arg) {
    size_t n = 0;

    for (; arg; arg >>= 1) {
        n += arg & 1;
    }

    return n;
}

inline size_t
count_flips (size_t n, size_t P) {
    return count_ones (size_t (((n >> 1) | ((n & 1) << (P - 1))) ^ n));
}

size_t
uniformity_measure (size_t n, size_t P) {
    const auto a = count_ones (n), b = count_flips (n, P);
    return b < 3 ? a : (P + 1);
}

} // namespace olbp_detail

////////////////////////////////////////////////////////////////////////

template< typename T, size_t R, size_t P >
/* explicit */ olbp_t< T, R, P >::olbp_t ()
{
    for (size_t i = 0; i < P; ++i) {
        N.emplace_back (
            nearbyint (-1.0 * R * sin (2. * M_PI * i / P)),
            nearbyint ( 1.0 * R * cos (2. * M_PI * i / P)));
    }
}

template< typename T, size_t R, size_t P >
inline size_t
olbp_t< T, R, P >::operator() (
    const cv::Mat& src, const cv::Mat& ref, size_t i, size_t j) const
{
    const auto& c = ref.at< T > (i, j);

    size_t n = 0, S = 0;

    for (const auto& off : N) {
        const auto& g = src.at< T > (i + off.first, j + off.second);
        n |= (size_t (c >= g) << S++);
    }

    return olbp_detail::uniformity_measure (n, P);
}

template< typename T, size_t R, size_t P >
inline cv::Mat
olbp_t< T, R, P >::operator() (const cv::Mat& src, const cv::Mat& ref) const
{
    LBP_ASSERT (src.size () == ref.size ());
    LBP_ASSERT (src.type () == ref.type ());

    auto dst = cv::Mat (src.size (), CV_8UC1, cv::Scalar (0));

    for (size_t i = R; i < src.rows - R; ++i) {
        for (size_t j = R; j < src.cols - R; ++j) {
            dst.at< unsigned char > (i, j) = this->operator() (src, ref, i, j);
        }
    }

    return dst;
}

template< typename T, size_t R, size_t P >
inline cv::Mat
olbp_t< T, R, P >::operator() (const cv::Mat& src) const
{
    return this->operator() (src, src);
}

template< typename T >
inline cv::Mat
olbp_t< T, 1, 8 >::operator() (const cv::Mat& src, const cv::Mat& ref) const
{
    LBP_ASSERT (src.size () == ref.size ());
    LBP_ASSERT (src.type () == ref.type ());

    static constexpr size_t U [] = {
        0, 1, 1, 2, 1, 9, 2, 3, 1, 9, 9, 9, 2, 9, 3, 4,
        1, 9, 9, 9, 9, 9, 9, 9, 2, 9, 9, 9, 3, 9, 4, 5,
        1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        2, 9, 9, 9, 9, 9, 9, 9, 3, 9, 9, 9, 4, 9, 5, 6,
        1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        2, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        3, 9, 9, 9, 9, 9, 9, 9, 4, 9, 9, 9, 5, 9, 6, 7,
        1, 2, 9, 3, 9, 9, 9, 4, 9, 9, 9, 9, 9, 9, 9, 5,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 6,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 7,
        2, 3, 9, 4, 9, 9, 9, 5, 9, 9, 9, 9, 9, 9, 9, 6,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 7,
        3, 4, 9, 5, 9, 9, 9, 6, 9, 9, 9, 9, 9, 9, 9, 7,
        4, 5, 9, 6, 9, 9, 9, 7, 5, 6, 9, 7, 6, 7, 7, 8
    };

    cv::Mat dst (src.size (), CV_8UC1);

    for (int i = 1; i < src.rows - 1; ++i) {
        const T* p = src.ptr< T > (i - 1);
        const T* q = src.ptr< T > (i);
        const T* r = src.ptr< T > (i + 1);

        T* s = dst.ptr< T > (i);

        const T* t = ref.ptr< T > (i);

        for (int j = 1; j < src.cols - 1; ++j) {
            const auto center = t [j];

#define T(a, b, c) ((center >= a [b]) << c)
            const unsigned char value =
                T (p, j - 1, 7) |
                T (p, j    , 6) |
                T (p, j + 1, 5) |
                T (r, j - 1, 1) |
                T (r, j    , 2) |
                T (r, j + 1, 3) |
                T (q, j - 1, 0) |
                T (q, j + 1, 4);
#undef T

            s [j] = U [value];
        }
    }

    return dst;
}

template< typename T >
inline cv::Mat
olbp_t< T, 1, 8 >::operator() (const cv::Mat& src) const
{
    return this->operator() (src, src);
}

template< typename T >
inline cv::Mat
olbp_t< T, 2, 12 >::operator() (const cv::Mat& src, const cv::Mat& ref) const
{
    LBP_ASSERT (src.size () == ref.size ());
    LBP_ASSERT (src.type () == ref.type ());

    cv::Mat dst (src.size (), CV_8UC1);

    for (int i = 2; i < src.rows - 2; ++i) {
        const T* p = src.ptr< T > (i - 2);
        const T* q = src.ptr< T > (i - 1);
        const T* r = src.ptr< T > (i);
        const T* s = src.ptr< T > (i + 1);
        const T* t = src.ptr< T > (i + 2);

        T* u = dst.ptr< T > (i);

        const T* w = ref.ptr< T > (i);

        for (int j = 2; j < src.cols - 2; ++j) {
            const auto center = w [j];

#define T(a, b, c) ((center >= a [b]) << c)
            const unsigned value =
                T (p, j - 1,  10) |
                T (p, j,       9) |
                T (p, j + 1,   8) |
                T (q, j - 2,  11) |
                T (q, j + 2,   7) |
                T (r, j - 2,   0) |
                T (r, j + 2,   6) |
                T (s, j - 2,   1) |
                T (s, j + 2,   5) |
                T (t, j - 1,   2) |
                T (t, j,       3) |
                T (t, j + 1,   4);
#undef T

            u [j] = olbp_detail::uniformity_measure (value, 12);
        }
    }

    return dst;
}

template< typename T >
inline cv::Mat
olbp_t< T, 2, 12 >::operator() (const cv::Mat& src) const
{
    return this->operator() (src, src);
}

template< typename T >
inline cv::Mat
olbp_t< T, 2, 16 >::operator() (const cv::Mat& src, const cv::Mat& ref) const
{
    LBP_ASSERT (src.size () == ref.size ());
    LBP_ASSERT (src.type () == ref.type ());

    cv::Mat dst (src.size (), CV_8UC1);

    for (int i = 2; i < src.rows - 2; ++i) {
        const T* p = src.ptr< T > (i - 2);
        const T* q = src.ptr< T > (i - 1);
        const T* r = src.ptr< T > (i);
        const T* s = src.ptr< T > (i + 1);
        const T* t = src.ptr< T > (i + 2);

        T* u = dst.ptr< T > (i);

        const T* w = ref.ptr< T > (i);

        for (int j = 2; j < src.cols - 2; ++j) {
            const auto center = w [j];

#define T(a, b, c) ((center >= a [b]) << c)
            const unsigned value =
                T (p, j - 1,  13) |
                T (p, j,      12) |
                T (p, j + 1,  11) |
                T (q, j - 2,  15) |
                T (q, j - 1,  14) |
                T (q, j + 1,  10) |
                T (q, j + 2,   9) |
                T (r, j - 2,   0) |
                T (r, j + 2,   8) |
                T (s, j - 2,   1) |
                T (s, j - 1,   2) |
                T (s, j + 1,   6) |
                T (s, j + 2,   7) |
                T (t, j - 1,   3) |
                T (t, j,       5) |
                T (t, j + 1,   4);
#undef T

            u [j] = olbp_detail::uniformity_measure (value, 16);
        }
    }

    return dst;
}

template< typename T >
inline cv::Mat
olbp_t< T, 2, 16 >::operator() (const cv::Mat& src) const
{
    return this->operator() (src, src);
}

} // namespace lbp
