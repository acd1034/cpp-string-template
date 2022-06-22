# Add include folders to the library and targets that consume it the SYSTEM
# keyword suppresses warnings for users of the library
if(STRTPL_STANDALONE_PROJECT)
  target_include_directories(StrTpl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
else()
  target_include_directories(StrTpl SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
endif()
