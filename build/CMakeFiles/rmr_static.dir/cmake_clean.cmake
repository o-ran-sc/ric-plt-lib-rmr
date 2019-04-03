file(REMOVE_RECURSE
  "librmr.pdb"
  "librmr.a"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/rmr_static.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
