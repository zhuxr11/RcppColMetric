if (require("MASS", quietly = TRUE) == TRUE) {
  data(cats)
  print(res_cpp <- col_mut_info(round(cats[, 2L:3L]), cats[, 1L]))
  # Validate with caTools::colAUC()
  if ((require("infotheo", quietly = TRUE) == TRUE) &&
      (require("magrittr", quietly = TRUE) == TRUE)) {
    print(res_r <- sapply(round(cats[, 2L:3L]), infotheo::mutinformation, cats[, 1L]) %>%
            {matrix(., nrow = 1L, dimnames = list(NULL, names(.)))})
    identical(res_cpp, res_r)
  }
}
