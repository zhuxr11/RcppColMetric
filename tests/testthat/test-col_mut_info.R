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
        testthat::expect_equal(
          col_mut_info_vec(list(round(cats[, 2L:3L])), list(cats[, 1L])),
          sapply(round(cats[, 2L:3L]), infotheo::mutinformation, cats[, 1L]) %>%
            {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))} %>%
            list()
        )
      }
    )
  }
}
