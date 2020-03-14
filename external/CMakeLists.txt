add_subdirectory (fmt)

SET (PEGTL_BUILD_EXAMPLES OFF CACHE BOOL "Build example programs" FORCE)
SET (PEGTL_BUILD_TESTS OFF CACHE BOOL "Build test programs" FORCE)
add_subdirectory (PEGTL)

option (USE_SYSTEM_GOOGLETEST "Use system package for GoogleTest" OFF)

if (USE_SYSTEM_GOOGLETEST)
   find_package (GTest REQUIRED)
else ()
   add_subdirectory (googletest)
endif ()

