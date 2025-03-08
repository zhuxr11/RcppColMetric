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
        # Error about x/y size mismatch
        testthat::expect_error(
          col_auc(cats[, 2L:3L], cats[1L:10L, 1L]),
          "length\\(y\\) and nrow\\(X\\) must be the same"
        )
        # Error about insufficient label levels
        testthat::expect_error(
          col_auc(cats[, 2L:3L], as.factor(rep("F", nrow(cats)))),
          "contain at least 2 class labels"
        )
        # Tests about directions
        testthat::expect_no_error(
          col_auc(cats[, 2L:3L], cats[, 1L], args = list(direction = ">"))
        )
        testthat::expect_no_error(
          col_auc(cats[, 2L:3L], cats[, 1L], args = list(direction = "<"))
        )
        # Tests about non-existing levels
        testthat::expect_no_error(
          col_auc(cats[, 2L:3L], factor(cats[, 1L], levels = c("F", "M", "<NA>")))
        )
        # Tests about vectorized function
        testthat::expect_equal(
          col_auc_vec(list(cats[, 2L:3L]), list(cats[, 1L])),
          list(caTools::colAUC(cats[, 2L:3L], cats[, 1L]))
        )
        # Recycle over x/y
        testthat::expect_equal(
          col_auc_vec(list(cats[, 2L:3L]), replicate(2L, cats[, 1L], simplify = FALSE)),
          replicate(2L, caTools::colAUC(cats[, 2L:3L], cats[, 1L]), simplify = FALSE)
        )
        testthat::expect_equal(
          col_auc_vec(list(cats[, 2L:3L]),
                      list(cats[, 1L]),
                      args = list(list(direction = "auto"))),
          list(caTools::colAUC(cats[, 2L:3L], cats[, 1L]))
        )
      }
    )
  }
}
