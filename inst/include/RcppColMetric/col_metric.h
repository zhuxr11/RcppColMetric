#include <Rcpp.h>
#include "utils.h"
using namespace Rcpp;

#ifndef RCPP_COLMETRIC_H_GEN_
#define RCPP_COLMETRIC_H_GEN_

namespace RcppColMetric
{
  template <int T1, int T2>
  class Metric
  {
  public:
    R_xlen_t output_dim;
    virtual Nullable<CharacterVector> row_names(const RObject& x, const Vector<T2>& y, const Nullable<List>& args = R_NilValue) const {
      return R_NilValue;
    };
    virtual NumericVector calc_col(const Vector<T1>& x, const Vector<T2>& y, const R_xlen_t& i, const Nullable<List>& args = R_NilValue) const = 0;
    virtual ~Metric() {}
  };

  template <int T1, int T2, int T3>
  inline Matrix<T3> col_metric(const RObject& x, const Vector<T2>& y, const Metric<T1, T2>& metric, const Nullable<List>& args = R_NilValue) {
    R_xlen_t n_feature = utils::get_feature_count(x);
    R_xlen_t n_sample = utils::get_sample_count(x);
    if (n_sample != y.length()) {
      stop("col_auc: length(y) and nrow(X) must be the same.");
    }
    // Derive comparisons
    Matrix<T3> out(metric.output_dim, n_feature);
    for (R_xlen_t feature_i = 0; feature_i < n_feature; feature_i++) {
      Vector<T1> feature_val = utils::slice_feature<T1>(x, feature_i);
      Vector<T3> out_vec = metric.calc_col(feature_val, y, feature_i, args);
      if (out_vec.length() != metric.output_dim) {
        stop("Length of vector from metric.calc_col() [%i] differs from metric.output_dim [%i]",
             out_vec.length(), metric.output_dim);
      }
      out(_, feature_i) = out_vec;
    }
    rownames(out) = metric.row_names(x, y, args);
    colnames(out) = utils::get_feature_names(x);
    return out;
  }

  template <int T1, int T2, int T3, typename T4>
  inline List col_metric_vec(
      const List& x,
      const List& y,
      T4 (*f)(const RObject&, const Vector<T2>&, const Nullable<List>&),
      const Nullable<List>& args = R_NilValue
  ) {
    R_xlen_t vec_len = utils::get_max_len(x.length(), y.length());
    List out(vec_len);
    for (R_xlen_t vec_i = 0; vec_i < vec_len; vec_i++) {
      RObject x_single = GETV(x, vec_i);
      Vector<T2> y_single = GETV(y, vec_i);
      Nullable<List> args_single = RcppColMetric::utils::get_args_single(args, vec_i);
      T4 metric_single = f(x_single, y_single, args_single);
      Matrix<T3> out_single = col_metric<T1, T2, T3>(x_single, y_single, metric_single, args_single);
      out(vec_i) = out_single;
    }
    return out;
  }
} // namespace: RcppColMetric

#endif // RCPP_COLMETRIC_H_GEN_
