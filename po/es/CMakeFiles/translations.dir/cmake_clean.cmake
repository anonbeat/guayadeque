FILE(REMOVE_RECURSE
  "CMakeFiles/translations"
  "guayadeque.mo"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/translations.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
