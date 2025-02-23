#include <Rcpp.h>
using namespace Rcpp;

#ifndef RCPP_COLMETRIC_MACROS
#define RCPP_COLMETRIC_MACROS
#define GETV(x, i) x(i % x.length()) // wrapped indexing of vector
#endif // RCPP_COLMETRIC_MACROS

#ifndef RCPP_COLMETRIC_UTILS_H_GEN_
#define RCPP_COLMETRIC_UTILS_H_GEN_

namespace RcppColMetric
{
  namespace utils
  {
    template <int T1>
    inline Vector<T1> slice_feature(const Matrix<T1>& x, const R_xlen_t& i) {
      return x(_, i);
    }

    template <int T1>
    inline R_xlen_t get_feature_count(const Matrix<T1>& x) {
      return x.ncol();
    }

    template <int T1>
    inline R_xlen_t get_sample_count(const Matrix<T1>& x) {
      return x.nrow();
    }

    template <int T1>
    inline CharacterVector get_feature_names(const Matrix<T1>& x) {
      return colnames(x);
    }

    template <int T1>
    inline Vector<T1> slice_feature(const DataFrame& x, const R_xlen_t& i) {
      return x(i);
    }

    inline R_xlen_t get_feature_count(const DataFrame& x) {
      return x.length();
    }

    inline R_xlen_t get_sample_count(const DataFrame& x) {
      return x.nrow();
    }

    inline CharacterVector get_feature_names(const DataFrame& x) {
      return x.names();
    }

    // Concatenate vectors
    template <int T1, typename T2>
    inline Vector<T1> concat_vec(const Vector<T1>& x, const Vector<T1>& y) {
      std::vector<T2> x_vec = as<std::vector<T2>>(x);
      std::vector<T2> y_vec = as<std::vector<T2>>(y);
      std::vector<T2> out(x_vec.size() + y_vec.size());
      std::copy(x_vec.begin(), x_vec.end(), out.begin());
      std::copy(y_vec.begin(), y_vec.end(), out.begin() + x_vec.size());
      return wrap(out);
    }

    inline bool find_name(const List& L, const std::string& s) {
      Rcpp::CharacterVector nv = L.names();
      for (int i=0; i<nv.size(); i++) {
        if (std::string(nv[i]) == s) {
          return true;
        }
      }
      return false;
    }

    inline R_xlen_t get_max_len(const R_xlen_t& x, const R_xlen_t& y) {
      if (x >= y) {
        return x;
      } else {
        return y;
      }
    }

    inline Nullable<List> get_args_single(const Nullable<List>& args, const R_xlen_t& i) {
      if (args.isNull() == true) {
        return R_NilValue;
      }
      List args_ = as<List>(args);
      List out = GETV(args_, i);
      return out;
    }
  } // namespace: utils
} // namespace: RcppColMetric

#endif // RCPP_COLMETRIC_UTILS_H_GEN_
