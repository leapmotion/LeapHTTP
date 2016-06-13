function(parse_version VERSION_VARIABLE VERSION)
  string(REPLACE "." ";" VERSION_LIST ${VERSION})
  list(LENGTH VERSION_LIST VERSION_LIST_LENGTH)
  list(GET VERSION_LIST 0 VERSION_MAJOR)
  list(GET VERSION_LIST 1 VERSION_MINOR)
  if(VERSION_LIST_LENGTH LESS 2)
    message(FATAL_ERROR "Version number ${VERSION_VARIABLE} is malformed")
  endif()
  if(VERSION_LIST_LENGTH GREATER 2)
    list(GET VERSION_LIST 2 VERSION_PATCH)
  else()
    set(VERSION_PATCH 0)
  endif()
  set(${VERSION_VARIABLE}_MAJOR ${VERSION_MAJOR} PARENT_SCOPE)
  set(${VERSION_VARIABLE}_MINOR ${VERSION_MINOR} PARENT_SCOPE)
  set(${VERSION_VARIABLE}_PATCH ${VERSION_PATCH} PARENT_SCOPE)
endfunction()