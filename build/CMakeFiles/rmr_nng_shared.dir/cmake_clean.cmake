file(REMOVE_RECURSE
  "librmr_nng.pdb"
  "librmr_nng.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/rmr_nng_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
