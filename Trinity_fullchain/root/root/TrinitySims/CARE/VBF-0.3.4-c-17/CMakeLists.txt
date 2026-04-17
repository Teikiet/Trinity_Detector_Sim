# CMakeLists file for VBF
# by Juniper Foote, 2023

cmake_minimum_required(VERSION 3.27)

project(
    VBF
    VERSION 0.3.4
    DESCRIPTION "VERITAS Bank File (VBF) library"
)

include(FindZLIB)
include(FindBZip2)

add_subdirectory(VBF)

set(XTRA_LIBS "")
set(LIBSS "")
set(LIBS "")

set(CMAKE_CXX_FLAGS "-g -O2 -fno-strict-aliasing ${CMAKE_CXX_FLAGS}")

set(Summary_GZIP "yes")
set(Summary_BZIP2 "yes")

if(ZLIB_FOUND)
  string(APPEND XTRA_LIBS "-lz ")
  string(APPEND LIBSS "${ZLIB_LIBRARIES} ")
  set(LIBS ${LIBS} ${ZLIB_LIBRARIES})
else()
  set(Summary_GZIP "no (zlib not found)")
endif()

if(BZIP2_FOUND)
  string(APPEND XTRA_LIBS "-lbz2 ")
  string(APPEND LIBSS "${BZIP2_LIBRARIES} ")
  set(LIBS ${LIBS} ${BZIP2_LIBRARIES})
else()
  set(Summary_BZIP2 "no (libbzip2 not found)")
endif()


target_include_directories(VBF PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(VBF_static PUBLIC ${CMAKE_SOURCE_DIR})
if(LIBS)
  target_link_libraries(VBF ${LIBS})
  target_link_libraries(VBF_static ${LIBS})
endif()

set(LLIBS ${LIBS} VBF_static)

# VBF Tools
add_executable(vbfCheck ${CMAKE_SOURCE_DIR}/vbfTools/vbfCheck.cpp)
target_link_libraries(vbfCheck ${LLIBS})
target_include_directories(vbfCheck PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(vbfConfig ${CMAKE_SOURCE_DIR}/vbfTools/vbfConfig.c)
target_link_libraries(vbfConfig ${LLIBS})
target_include_directories(vbfConfig PUBLIC ${CMAKE_SOURCE_DIR})

set(q " \"")
set(rq "\" ")
target_compile_definitions(
  vbfConfig PRIVATE 
  PREFIX=${q}${CMAKE_INSTALL_PREFIX}${rq}
	INCLUDE_DIR=${q}${CMAKE_INSTALL_PREFIX}/include${rq}
	LIB_DIR=${q}${CMAKE_INSTALL_PREFIX}/lib${rq}
	XTRA_LDFLAGS=${q}${rq}
	XTRA_CPPFLAGS=${q}${rq}
	XTRA_LIBS=${q}${XTRA_LIBS}${rq}
	CXXFLAGS=${q}${CMAKE_CXX_FLAGS}${rq}
	CPPFLAGS=${q}${CMAKE_C_FLAGS}${rq}
	LDFLAGS=${q}${LDFLAGS}${rq}
	LIBS=${q}${rq}
	PACKAGE_STRING="VBF ${PROJECT_VERSION}"
)

add_executable(vbfList ${CMAKE_SOURCE_DIR}/vbfTools/vbfList.cpp)
target_link_libraries(vbfList ${LLIBS})
target_include_directories(vbfList PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(vbfPed  ${CMAKE_SOURCE_DIR}/vbfTools/vbfPed.cpp)
target_link_libraries(vbfPed ${LLIBS})
target_include_directories(vbfPed PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(vbfSummary  ${CMAKE_SOURCE_DIR}/vbfTools/vbfSummary.cpp)
target_link_libraries(vbfSummary ${LLIBS})
target_include_directories(vbfSummary PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(vbfRepTel  ${CMAKE_SOURCE_DIR}/vbfTools/vbfRepTel.cpp)
target_link_libraries(vbfRepTel ${LLIBS})
target_include_directories(vbfRepTel PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(vbfReindex  ${CMAKE_SOURCE_DIR}/vbfTools/vbfReindex.cpp)
target_link_libraries(vbfReindex ${LLIBS})
target_include_directories(vbfReindex PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(vbfZip  ${CMAKE_SOURCE_DIR}/vbfTools/vbfZip.cpp)
target_link_libraries(vbfZip ${LLIBS})
target_include_directories(vbfZip PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(vbfZeroSup  ${CMAKE_SOURCE_DIR}/vbfTools/vbfZeroSup.cpp)
target_link_libraries(vbfZeroSup ${LLIBS})
target_include_directories(vbfZeroSup PUBLIC ${CMAKE_SOURCE_DIR})

message(STATUS "VBF Configuration Summary:")
message(STATUS "                Binary directory: '${CMAKE_INSTALL_PREFIX}/bin'")
message(STATUS "                Library directory: '${CMAKE_INSTALL_PREFIX}/lib'")
message(STATUS "                Include directory: '${CMAKE_INSTALL_PREFIX}/include'")
message(STATUS "                zlib support: ${Summary_GZIP}")
message(STATUS "                libbzip2 support: ${Summary_BZIP2}")

configure_file(${CMAKE_SOURCE_DIR}/VBF_cmake.pc.in ${CMAKE_SOURCE_DIR}/VBF.pc @ONLY)

set_target_properties(VBF_static PROPERTIES OUTPUT_NAME VBF)

set_property(TARGET VBF VBF_static vbfCheck vbfConfig vbfList vbfPed vbfSummary vbfRepTel vbfReindex vbfZip vbfZeroSup PROPERTY CXX_STANDARD 17)
set_property(TARGET VBF VBF_static vbfCheck vbfConfig vbfList vbfPed vbfSummary vbfRepTel vbfReindex vbfZip vbfZeroSup PROPERTY CXX_STANDARD_REQUIRED ON)

install(TARGETS VBF VBF_static vbfCheck vbfConfig vbfList vbfPed vbfSummary vbfRepTel vbfReindex vbfZip vbfZeroSup)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/VBF" # source directory
    DESTINATION "include" # target directory
    FILES_MATCHING # install only matched files
    PATTERN "*.h" # select header files
)

install(
    FILES "VBF.pc"
    DESTINATION "lib/pkgconfig" # target directory
)

install(
  FILES "scripts/vbfCalcThreshold" "scripts/vbfCalcThreshold2" "scripts/vbfCalcThreshold3"
  DESTINATION "bin"
)
