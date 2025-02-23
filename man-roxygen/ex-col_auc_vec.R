if (require("MASS", quietly = TRUE) == TRUE) {
  data(cats)
  print(res_cpp <- col_auc_vec(list(cats[, 2L:3L]), list(cats[, 1L])))
  # Validate with caTools::colAUC()
  if (require("caTools", quietly = TRUE) == TRUE) {
    print(res_r <- caTools::colAUC(cats[, 2L:3L], cats[, 1L]))
    identical(res_cpp, list(res_r))
  }
}
