INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB FREEIMAGE_PLUS_SOURCES FreeImagePlus/src/*.cpp FreeImagePlus/FreeImagePlus.h)

IF(FREEIMAGE_BUILD_STATIC)
  add_library(FreeImagePlus_static STATIC ${FREEIMAGE_PLUS_SOURCES})
  target_link_libraries(FreeImagePlus_static FreeImage_static)
  target_include_directories(FreeImagePlus_static PUBLIC FreeImagePlus)
ENDIF()
