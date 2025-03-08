library(testthat)

if (require(MASS, quietly = TRUE) == TRUE) {
  data(cats)

  if ((require(infotheo, quietly = TRUE) == TRUE) &&
      (require(magrittr, quietly = TRUE) == TRUE)) {
    testthat::test_that(
      "Testing col_mut_info() ...", {
        testthat::expect_equal(
          col_mut_info(round(cats[, 2L:3L]), cats[, 1L]),
          sapply(round(cats[, 2L:3L]), infotheo::mutinformation, cats[, 1L]) %>%
            {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))}
        )
        # Tests with different methods
        method_vec <- c("emp", "mm", "sg", "shrink")
        invisible(sapply(
          seq_along(method_vec),
          function(method_idx) {
            testthat::expect_equal(
              col_mut_info(round(cats[, 2L:3L]), cats[, 1L], args = list(method = method_idx - 1L)),
              sapply(round(cats[, 2L:3L]), infotheo::mutinformation, cats[, 1L], method = method_vec[method_idx]) %>%
                {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))}
            )
          }
        ))
        # Error about length mismatch
        testthat::expect_error(
          col_mut_info(round(cats[, 2L:3L]), cats[1L:10L, 1L]),
          "length\\(y\\) and nrow\\(X\\) must be the same"
        )
        # Tests about vectorized function
        testthat::expect_equal(
          col_mut_info_vec(list(round(cats[, 2L:3L])), list(cats[, 1L])),
          sapply(round(cats[, 2L:3L]), infotheo::mutinformation, cats[, 1L]) %>%
            {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))} %>%
            list()
        )
        # Tests about vectorized function with method selection
        testthat::expect_equal(
          col_mut_info_vec(list(round(cats[, 2L:3L])), list(cats[, 1L]), args = list(list(method = 2L - 1L))),
          sapply(round(cats[, 2L:3L]), infotheo::mutinformation, cats[, 1L], method = method_vec[2L]) %>%
            {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))} %>%
            list()
        )
      }
    )
  }
}
