library(testthat)

if (require(MASS, quietly = TRUE) == TRUE) {
  data(cats)

  if (require(caTools, quietly = TRUE) == TRUE) {
    testthat::test_that(
      "Testing col_auc() ...", {
        testthat::expect_equal(
          col_auc(cats[, 2L:3L], cats[, 1L]),
          caTools::colAUC(cats[, 2L:3L], cats[, 1L])
        )
        testthat::expect_equal(
          col_auc_vec(list(cats[, 2L:3L]), list(cats[, 1L])),
          list(caTools::colAUC(cats[, 2L:3L], cats[, 1L]))
        )
      }
    )
  }
}
