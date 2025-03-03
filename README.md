
<!-- README.md is generated from README.Rmd. Please edit that file -->

# RcppColMetric

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://lifecycle.r-lib.org/articles/stages.html#experimental)
[![CRAN
status](https://www.r-pkg.org/badges/version/RcppColMetric)](https://CRAN.R-project.org/package=RcppColMetric)
[![R-CMD-check](https://github.com/zhuxr11/RcppColMetric/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/zhuxr11/RcppColMetric/actions/workflows/R-CMD-check.yaml)
[![Download
stats](https://cranlogs.r-pkg.org/badges/grand-total/RcppColMetric)](https://CRAN.R-project.org/package=RcppColMetric)
<!-- badges: end -->

**Package**: [*RcppColMetric*](https://github.com/zhuxr11/RcppColMetric)
0.0.0.9000<br /> **Author**: Xiurui Zhu<br /> **Modified**: 2025-03-03
18:11:42<br /> **Compiled**: 2025-03-03 18:11:49

The goal of `RcppColMetric` is to efficiently compute metrics between
various vectors and a common vector. This is common in data science,
such as computing performance metrics between each feature and a common
response. [`Rcpp`](https://CRAN.R-project.org/package=Rcpp) is used to
efficiently iterate over vectors through compiled code. You may extend
its utilities by providing custom metrics that fit into the framework.

## Installation

You can install the released version of `RcppColMetric` from
[CRAN](https://cran.r-project.org/) with:

``` r
install.packages("RcppColMetric")
```

Alternatively, you can install the developmental version of
`RcppColMetric` from [github](https://github.com/) with:

``` r
remotes::install_github("zhuxr11/RcppColMetric")
```

## Examples

``` r
library(cbbinom)
```

We use `cats` from [`MASS`](https://CRAN.R-project.org/package=MASS) to
illustrate the use of the package.

``` r
library(MASS)
data(cats)
print(head(cats))
#>   Sex Bwt Hwt
#> 1   F 2.0 7.0
#> 2   F 2.0 7.4
#> 3   F 2.0 9.5
#> 4   F 2.1 7.2
#> 5   F 2.1 7.3
#> 6   F 2.1 7.6
```

### Column-wise ROC-AUC

In binary classification modelling, it is a common practice to compute
ROC-AUC of each feature (usually columns) against a common target.
`RcppColMetric` provides a much faster version than its commonly used
counterparts, e.g. `caTools::colAUC()`.

``` r
library(caTools)
(col_auc_bench <- microbenchmark::microbenchmark(
  col_auc_r = caTools::colAUC(cats[, 2L:3L], cats[, 1L]),
  col_auc_cpp = col_auc(cats[, 2L:3L], cats[, 1L]),
  times = 100L,
  check = "identical"
))
#> Unit: microseconds
#>         expr   min     lq   mean median    uq    max neval
#>    col_auc_r 523.0 554.85 620.06  621.7 653.9 1031.3   100
#>  col_auc_cpp 128.3 147.30 165.06  161.3 175.1  287.0   100
```

As can be seen, the median speed of computation from `RcppColMetric` is
3.854 times faster.

If there are multiple sets of features and responses, you may use the
vectorized version `col_auc_vec()`, which uses compiled code to speed up
iterations and returns a list.

``` r
col_auc_vec(list(cats[, 2L:3L]), list(cats[, 1L]))
#> [[1]]
#>               Bwt      Hwt
#> F vs. M 0.8338451 0.759048
```

### Column-wize mutual information

In classification modelling, it is another common practice to assess
mutual information between features and a response if the features are
discrete. `RcppColMetric` provides a much faster version than its
commonly used counterparts, e.g. `infotheo::mutinformation()`.

``` r
library(infotheo)
(col_mut_info_bench <- microbenchmark::microbenchmark(
  col_mut_info_r = sapply(round(cats[, 2L:3L]), infotheo::mutinformation, cats[, 1L]) %>%
    {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))},
  col_mut_info_cpp = col_mut_info(round(cats[, 2L:3L]), cats[, 1L]),
  times = 100L,
  check = "identical"
))
#> Unit: microseconds
#>              expr    min      lq     mean  median      uq     max neval
#>    col_mut_info_r 1638.4 1737.05 2057.062 1856.95 1966.75 13168.5   100
#>  col_mut_info_cpp  361.1  389.25  449.926  422.55  474.70  1190.1   100
```

As can be seen, the median speed of computation from `RcppColMetric` is
4.395 times faster.

If there are multiple sets of features and responses, you may use the
vectorized version `col_mut_info_vec()`, which uses compiled code to
speed up iterations and returns a list.

``` r
col_mut_info_vec(list(round(cats[, 2L:3L])), list(cats[, 1L]))
#> [[1]]
#>            Bwt       Hwt
#> [1,] 0.1346783 0.1620514
```

## Extend the package with custom metric

You may implement your own metric by inheriting from
`RcppColMetric::Metric` class with template arguments as feature `SEXP`
(input, numeric here) and response `SEXP` (input, factor as integer
here) types. For example, to compute range of each feature, define a
`RangeMetric` class.

``` cpp
#include <RcppColMetric.h>
#include <Rcpp.h>
using namespace Rcpp;


// x: numeric (REALSXP), y: factor -> integer (INTSXP), output: numeric (REALSXP)
class 
RangeMetric: public RcppColMetric::Metric<REALSXP, INTSXP, REALSXP>
{
public:
  // Constructor
  RangeMetric(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) {
    // This parameter is inherited from `Metric`, determining output dimension (number of rows)
    // For RangeMetric, the output dimension is 2 (min & max)
    output_dim = 2;
  }
  virtual Nullable<CharacterVector> row_names(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) const override {
    // Determine the row names
    // If not used, it may return R_NilValue
    CharacterVector out = {"min", "max"};
    return out;
  }
  virtual NumericVector calc_col(const NumericVector& x, const IntegerVector& y, const R_xlen_t& i, const Nullable<List>& args = R_NilValue) const override {
    // Derive output value for each feature and the common response
    // For RangeMetric, the output is min & max
    NumericVector out = {min(x), max(x)};
    return out;
  }
};
```

Then, define the main function calling `RcppColMetric::col_metric()`,
with corresponding feature `SEXP` (input, numeric here), response `SEXP`
(input, factor as integer here) and output `SEXP` (output, numeric here)
types.

``` cpp
NumericMatrix col_range(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) {
  RangeMetric range_metric(x, y, args);
  NumericMatrix out = RcppColMetric::col_metric<REALSXP, INTSXP, REALSXP>(x, y, range_metric, args);
  return out;
}
```

Test this function with `cats`:

``` r
col_range(cats[, 2L:3L], cats[, 1L])
#>     Bwt  Hwt
#> min 2.0  6.3
#> max 3.9 20.5
```

To define vectorized version of the function, a wrapper function is
defined to generate `RangeMetric` object (taking only `x`, `y` and
`args`), and then passed on to the workhorse
`RcppColMetric::col_metric_vec()`.

``` cpp
RangeMetric gen_range_metric(const RObject& x, const IntegerVector& y, const Nullable<List>& args = R_NilValue) {
  RangeMetric out(x, y, args);
  return out;
}

// [[Rcpp::export]]
List col_range_vec(const List& x, const List& y, const Nullable<List>& args = R_NilValue) {
  List out = RcppColMetric::col_metric_vec<REALSXP, INTSXP, REALSXP>(x, y, &gen_range_metric, args);
  return out;
}
```

Test the vectorized function with `cats`:

``` r
col_range_vec(list(cats[, 2L:3L]), list(cats[, 1L]))
#> [[1]]
#>     Bwt  Hwt
#> min 2.0  6.3
#> max 3.9 20.5
```
