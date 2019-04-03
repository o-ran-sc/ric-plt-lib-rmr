file(REMOVE_RECURSE
  "librmr.pdb"
  "librmr.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/rmr_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
