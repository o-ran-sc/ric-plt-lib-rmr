file(REMOVE_RECURSE
  "librmr_nng.pdb"
  "librmr_nng.a"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/rmr_nng_static.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
