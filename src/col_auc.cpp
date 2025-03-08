#include <Rcpp.h>
#include "../inst/include/RcppColMetric.h"
using namespace Rcpp;

// Derive pairwise comparisons
List pair_comp(const CharacterVector& x) {
  List out(x.length() * (x.length() - 1) / 2);
  R_xlen_t cur_idx = 0;
  for (R_xlen_t lvl_i = 0; lvl_i < x.length() - 1; lvl_i++) {
    for (R_xlen_t lvl_j = lvl_i + 1; lvl_j < x.length(); lvl_j++) {
      CharacterVector x_subset = {x(lvl_i), x(lvl_j)};
      out(cur_idx) = x_subset;
      cur_idx++;
    }
  }
  return out;
}

// Derive rank (average ties): Rcpp rank function that does average ties
class Comparator {
private:
  const Rcpp::NumericVector& ref;
  bool is_na(double x) const
  {
    return Rcpp::traits::is_na<REALSXP>(x);
  }
public:
  Comparator(const Rcpp::NumericVector& ref_)
    : ref(ref_)
  {}
  bool operator()(const int ilhs, const int irhs) const
  {
    double lhs = ref[ilhs], rhs = ref[irhs];
    if (is_na(lhs)) return false;
    if (is_na(rhs)) return true;
    return lhs < rhs;
  }
};
NumericVector avg_rank(const NumericVector& x)
{
  R_xlen_t sz = x.size();
  IntegerVector w = seq(0, sz - 1);
  std::sort(w.begin(), w.end(), Comparator(x));
  NumericVector r = no_init_vector(sz);
  for (R_xlen_t n, i = 0; i < sz; i += n) {
    n = 1;
    while (i + n < sz && x[w[i]] == x[w[i + n]]) ++n;
    for (R_xlen_t k = 0; k < n; k++) {
      r[w[i + k]] = i + (n + 1) / 2.;
    }
  }
  return r;
}

class AucMetric: public RcppColMetric::Metric<REALSXP, INTSXP, REALSXP>
{
public:
  CharacterVector y_level;
  R_xlen_t n_level;
  List comp_list;
  List idx_list;
  String name_sep;
  AucMetric(const RObject& x, const IntegerVector& y, const String name_sep_, const Nullable<List>& args = R_NilValue): name_sep(name_sep_) {
    y_level = y.attr("levels");
    n_level = y_level.length();
    if (n_level < 2) {
      stop("col_auc: List of labels 'y' have to contain at least 2 class labels.");
    }
    // Derive output dimension
    output_dim = n_level * (n_level - 1) / 2;
    // Derive comparisons
    comp_list = pair_comp(y_level);
    // Separate values in x for each level in y
    List idx_list_(n_level);
    idx_list_.attr("names") = y_level;
    IntegerVector idx_template = seq(0, y.length() - 1);
    for (R_xlen_t idx_i = 0; idx_i < n_level; idx_i++) {
      idx_list_(idx_i) = idx_template[y == idx_i + 1];
    }
    idx_list = clone(idx_list_);
  }
  virtual Nullable<CharacterVector> row_names(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) const override {
    CharacterVector comp_name(comp_list.length());
    for (R_xlen_t comp_i = 0; comp_i < comp_list.length(); comp_i++) {
      CharacterVector comp_level = comp_list(comp_i);
      String comp_level_from = comp_level(0);
      String comp_level_to = comp_level(1);
      String comp_name_single(comp_level_from);
      comp_name_single += name_sep;
      comp_name_single += comp_level_to;
      comp_name(comp_i) = comp_name_single;
    }
    return comp_name;
  }
  virtual NumericVector calc_col(const NumericVector& x, const IntegerVector& y, const R_xlen_t& i, const Nullable<List>& args = R_NilValue) const override {
    // Apply Wilcoxon algorithm
    NumericVector out(output_dim);
    for (R_xlen_t comp_i = 0; comp_i < comp_list.length(); comp_i++) {
      CharacterVector comp_level = comp_list(comp_i);
      String comp_level_from = comp_level(0);
      IntegerVector idx_from = idx_list[comp_level_from];
      String comp_level_to = comp_level(1);
      IntegerVector idx_to = idx_list[comp_level_to];
      R_xlen_t n_from = idx_from.length();
      R_xlen_t n_to = idx_to.length();
      if (n_from > 0 && n_to > 0) {
        NumericVector feature_val_comp = RcppColMetric::utils::concat_vec<REALSXP, double>(x[idx_from], x[idx_to]);
        NumericVector feature_val_rank = avg_rank(feature_val_comp);
        NumericVector feature_val_rank_from = feature_val_rank[seq(0, n_from - 1)];
        double auc = (sum(feature_val_rank_from) - n_from * (n_from + 1) / 2) / (n_from * n_to);
        String direction = "auto";
        if (args.isNotNull() == true) {
          List args_ = as<List>(args);
          if (RcppColMetric::utils::find_name(args_, "direction") == true) {
            CharacterVector direction_vec = args_["direction"];
            direction = GETV(direction_vec, i);
          }
        }
        if (direction == ">") {
          out(comp_i) = auc;
        } else if (direction == "<") {
          out(comp_i) = 1 - auc;
        } else {
          NumericVector auc_vec = {auc, 1 - auc};
          out(comp_i) = max(auc_vec);
        }
      } else {
        out(comp_i) = NA_REAL;
      }
    }
    return out;
  }
};

//' Column-wise area under ROC curve (AUC)
//'
//' Calculate area under the ROC curve (AUC) for every column of a matrix or data frame. For better performance, data frame is preferred.
//'
//' @param x Matrix or data frame. Rows contain samples and columns contain features/variables.
//' @param y Factor of class labels for the data samples. A response vector with one label for each row/component of \code{x}.
//' @param args \code{NULL} (default) or list of named arguments: \describe{
//' \item{direction}{Character vector containing one of the following directions: \code{">"}, \code{"<"} or \code{"auto"} (default),
//' recycled for each feature so different directions can be used for different features.}
//' }
//'
//' @return An output is a single matrix with the same number of columns as X and "n choose 2" ( n!/((n-2)! 2!) = n(n-1)/2 ) number of rows,
//' where n is number of unique labels in y list. For example, if y contains only two unique class labels ( length(unique(lab))==2 )
//' then output matrix will have a single row containing AUC of each column.
//' If more than two unique labels are present than AUC is calculated for every possible pairing of classes ("n choose 2" of them).
//'
//' @note Change log:
//' \itemize{
//'   \item{0.1.0 Xiurui Zhu - Initiate the function.}
//' }
//'
//' @export
//' @seealso \code{caTools::colAUC} for the original \R implementation.
//' @seealso \code{\link{col_auc_vec}} for the vectorized version.
//' @example man-roxygen/ex-col_auc.R
// [[Rcpp::export]]
NumericMatrix col_auc(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) {
 AucMetric auc_metric(x, y, " vs. ", args);
 NumericMatrix out = RcppColMetric::col_metric<REALSXP, INTSXP, REALSXP>(x, y, auc_metric, args);
 return out;
}

AucMetric gen_auc_metric(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) {
  AucMetric out(x, y, " vs. ", args);
  return out;
}

//' @templateVar fun_name col_auc
//' @template template-vec_function
//'
//' @note Change log:
//' \itemize{
//'   \item{0.1.0 Xiurui Zhu - Initiate the function.}
//' }
//'
//' @export
//' @example man-roxygen/ex-col_auc_vec.R
// [[Rcpp::export]]
List col_auc_vec(const List& x, const List& y, const Nullable<List>& args = R_NilValue) {
  List out = RcppColMetric::col_metric_vec<REALSXP, INTSXP, REALSXP>(x, y, &gen_auc_metric, args);
  return out;
}

// You can include R code blocks in C++ files processed with sourceCpp
// (useful for testing and development). The R code will be automatically
// run after the compilation.
//
/*** R
library(MASS)
data(cats)
microbenchmark::microbenchmark(
  col_auc_r_mat = caTools::colAUC(as.matrix(cats[, 2L:3L]), cats[, 1L]),
  col_auc_r_df = caTools::colAUC(cats[, 2L:3L], cats[, 1L]),
  col_auc_cpp_mat = col_auc(as.matrix(cats[, 2L:3L]), cats[, 1L]),
  col_auc_cpp_df = col_auc(cats[, 2L:3L], cats[, 1L]),
  times = 100L,
  check = "identical"
)
*/

