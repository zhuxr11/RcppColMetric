#include <Rcpp.h>
#include <R.h>
#include <Rdefines.h>
#include "../inst/include/RcppColMetric.h"
using namespace Rcpp;

// These functions are copied from: https://github.com/cran/infotheo/blob/master/src/entropy.cpp @4c12f5610dc2e204e7d9000c92a4057dc069c405

double entropy_empirical(std::map< std::vector<int> ,int > frequencies, int nb_samples) {
  double e = 0;
  for (std::map< std::vector<int> ,int>::const_iterator iter = frequencies.begin(); iter != frequencies.end(); ++iter)
    e -= iter->second * log((double)iter->second);
  return log((double)nb_samples) + e/nb_samples;
}

double entropy_miller_madow(std::map< std::vector<int> ,int > frequencies, int nb_samples) {
  return entropy_empirical(frequencies,nb_samples) + (int(frequencies.size())-1)/(2.0*nb_samples);
}

double entropy_dirichlet(std::map< std::vector<int> ,int > frequencies, int nb_samples, double beta) {
  double e = 0;
  for (std::map< std::vector<int> ,int>::const_iterator iter = frequencies.begin(); iter != frequencies.end(); ++iter)
    e+=(iter->second+beta)*(R::digamma(nb_samples+(frequencies.size()*beta)+1)-R::digamma(iter->second+beta+1));
  return e/(nb_samples+(frequencies.size()*beta));
}

double entropy_shrink(std::map< std::vector<int> ,int > frequencies, int nb_samples)
{
  double w = 0;
  int p = frequencies.size(), n2 = nb_samples*nb_samples;
  double lambda, beta;
  for (std::map< std::vector<int> ,int>::const_iterator iter = frequencies.begin(); iter != frequencies.end(); ++iter)
    w += iter->second*iter->second;
  lambda = p*(n2 - w)/((nb_samples-1)*(w*p - n2));
  if(lambda >= 1)
    return -log(1.0/p);
  else {
    beta = (lambda/(1-lambda))*nb_samples/frequencies.size();
    return entropy_dirichlet(frequencies, nb_samples, beta);
  }
}

double entropy(const int *d, int nsamples, int nvars, int c, bool *v) {
  // H(d) using estimator c
  std::map< std::vector<int> ,int > freq;
  std::vector<int> sel;
  bool ok = true;
  int nsamples_ok = 0;
  double H = 0;
  for(int s = 0; s < nsamples; ++s) {
    ok = true;
    sel.clear();
    for(int i = 0; i < nvars; ++i) {
      if(v[i]){
        if(d[s+i*nsamples]!=NA_INTEGER)
          sel.push_back(d[s+i*nsamples]);
        else
          ok = false;
      }
    }
    if(ok) {
      freq[sel]++;
      nsamples_ok++;
    }
  }
  if( c == 0 ) //empirical
    H = entropy_empirical(freq,nsamples_ok);
  else if( c == 1 ) //miller-madow
    H = entropy_miller_madow(freq,nsamples_ok);
  else if( c == 2 ) //dirichlet Schurmann-Grassberger
    H = entropy_dirichlet(freq,nsamples_ok, 1/freq.size());
  else if( c == 3 ) // shrink
    H = entropy_shrink(freq,nsamples_ok);
  return H;
}

SEXP entropyR (SEXP Rdata, SEXP Rnrows, SEXP Rncols, SEXP Rchoice)
{
  const int *data;
  const int *nrows, *ncols, *choice;
  SEXP res;
  PROTECT(Rdata = AS_INTEGER(Rdata));
  PROTECT(Rnrows= AS_INTEGER(Rnrows));
  PROTECT(Rncols= AS_INTEGER(Rncols));
  PROTECT(Rchoice= AS_INTEGER(Rchoice));
  data = INTEGER_POINTER(Rdata);
  nrows= INTEGER_POINTER(Rnrows);
  ncols= INTEGER_POINTER(Rncols);
  choice= INTEGER_POINTER(Rchoice);
  PROTECT(res = NEW_NUMERIC(1));
  bool *sel = new bool[*ncols];
  for( int i=0; i<*ncols; ++i )
    sel[i] = true;
  REAL(res)[0] = entropy(data, *nrows, *ncols, *choice, sel);
  UNPROTECT(5);
  return res;
}

// Compute mutual information from entrtopies
double mut_info(const IntegerVector& x, const IntegerVector& y, const int& method) {
  NumericVector entropy_x = entropyR(x, wrap(x.length()), wrap(1), wrap(method));
  NumericVector entropy_y = entropyR(y, wrap(y.length()), wrap(1), wrap(method));
  IntegerVector xy = RcppColMetric::utils::concat_vec<INTSXP, int>(x, y);
  NumericVector entropy_xy = entropyR(xy, wrap(x.length()), wrap(2), wrap(method));
  NumericVector out = entropy_x + entropy_y - entropy_xy;
  return out(0);
}

class MutInfoMetric: public RcppColMetric::Metric<INTSXP, INTSXP, REALSXP>
{
public:
  int method;
  MutInfoMetric(const RObject& x, const IntegerVector& y, const int& method_, const Nullable<List>& args = R_NilValue): method(method_) {
    output_dim = 1;
  }
  virtual NumericVector calc_col(const IntegerVector& x, const IntegerVector& y, const R_xlen_t& i, const Nullable<List>& args = R_NilValue) const override {
    double res = mut_info(x, y, method);
    NumericVector out(output_dim, res);
    return out;
  }
};

//' Column-wise mutual information
//'
//' Calculate mutual information for every column of a matrix or data frame. Only discrete values are allowed.
//' For better performance, data frame is preferred.
//'
//' @param x Matrix or data frame of discrete values (integers). Rows contain samples and columns contain features/variables.
//' @param y Factor of class labels for the data samples. A response vector with one label for each row/component of \code{x}.
//' @param args \code{NULL} (default) or list of named arguments: \describe{
//' \item{method}{Integer indicating computation method: 0 = empirical, 1 = Miller-Madow,
//' 2 = shrink, 3 = Schurmann-Grassberger.}
//' }
//'
//' @return An output is a single matrix with the same number of columns as X and 1 row.
//'
//' @note Change log:
//' \itemize{
//'   \item{0.1.0 Xiurui Zhu - Initiate the function.}
//' }
//'
//' @export
//' @seealso \code{infotheo::mutinformation} for the original computation
//' of mutual information in \R (and also the computation methods).
//' @seealso \code{\link{col_mut_info_vec}} for the vectorized version.
//' @example man-roxygen/ex-col_mut_info.R
// [[Rcpp::export]]
NumericMatrix col_mut_info(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) {
  int method = 0;
  if (args.isNotNull() == true) {
    List args_ = as<List>(args);
    if (RcppColMetric::utils::find_name(args_, "method") == true) {
      method = args_["method"];
    }
  }
  MutInfoMetric mut_info_metric(x, y, method, args);
  NumericMatrix out = RcppColMetric::col_metric<INTSXP, INTSXP, REALSXP>(x, y, mut_info_metric, args);
  return out;
}

MutInfoMetric gen_mut_info_metric(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) {
  int method = 0;
  if (args.isNotNull() == true) {
    List args_ = as<List>(args);
    if (RcppColMetric::utils::find_name(args_, "method") == true) {
      method = args_["method"];
    }
  }
  MutInfoMetric out(x, y, method, args);
  return out;
}

//' @templateVar fun_name col_mut_info
//' @template template-vec_function
//'
//' @note Change log:
//' \itemize{
//'   \item{0.1.0 Xiurui Zhu - Initiate the function.}
//' }
//'
//' @export
//' @example man-roxygen/ex-col_mut_info_vec.R
// [[Rcpp::export]]
List col_mut_info_vec(const List& x, const List& y, const Nullable<List>& args = R_NilValue) {
  List out = RcppColMetric::col_metric_vec<INTSXP, INTSXP, REALSXP>(x, y, &gen_mut_info_metric, args);
  return out;
}

// You can include R code blocks in C++ files processed with sourceCpp
// (useful for testing and development). The R code will be automatically
// run after the compilation.
//

/*** R
library(MASS)
library(magrittr)
data(cats)
microbenchmark::microbenchmark(
  col_mut_info_r_mat = apply(as.matrix(round(cats[, 2L:3L])), 2L, infotheo::mutinformation, cats[, 1L]) %>%
    {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))},
  col_mut_info_r_df = sapply(round(cats[, 2L:3L]), infotheo::mutinformation, cats[, 1L]) %>%
    {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))},
  col_mut_info_cpp_mat = col_mut_info(as.matrix(round(cats[, 2L:3L])), cats[, 1L]),
  col_mut_info_cpp_df = col_mut_info(round(cats[, 2L:3L]), cats[, 1L]),
  times = 100L,
  check = "identical"
)
*/
