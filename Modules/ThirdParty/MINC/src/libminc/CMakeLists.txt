# CMakeFiles.txt for the libminc library
#
# Andrew Janke - a.janke@gmail.com
# Vladimir S. FONOV - vladimir.fonov@gmail.com

PROJECT(LIBMINC)

SET(LIBMINC_PACKAGE_VERSION_MAJOR 2)
SET(LIBMINC_PACKAGE_VERSION_MINOR 4)
SET(LIBMINC_PACKAGE_VERSION_PATCH 06)

SET(LIBMINC_SOVERSION "5.3.0")

SET(LIBMINC_PACKAGE "libminc")
SET(LIBMINC_PACKAGE_BUGREPORT "vladimir.fonov@gmail.com")

SET(LIBMINC_PACKAGE_NAME    "libminc")
SET(LIBMINC_PACKAGE_VERSION "${LIBMINC_PACKAGE_VERSION_MAJOR}.${LIBMINC_PACKAGE_VERSION_MINOR}.${LIBMINC_PACKAGE_VERSION_PATCH}")
SET(LIBMINC_PACKAGE_STRING  "${LIBMINC_PACKAGE_NAME} ${LIBMINC_PACKAGE_VERSION}")

INCLUDE(CTest)

ENABLE_TESTING()

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

IF( POLICY CMP0063 )
  CMAKE_POLICY(SET CMP0063 NEW)
ENDIF()

IF(MINC_TOOLKIT_BUILD)
  SET(LIBMINC_EXTERNALLY_CONFIGURED ON)
ENDIF(MINC_TOOLKIT_BUILD)

SET(LIBMINC_BUILD_V2 ON)

IF(NOT LIBMINC_EXTERNALLY_CONFIGURED)
  # Allowing User to configure parameters
  SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

  SET(CPACK_GENERATOR TGZ)
  SET(CPACK_PACKAGE_VERSION_MAJOR ${LIBMINC_PACKAGE_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR ${LIBMINC_PACKAGE_VERSION_MINOR})
  SET(CPACK_PACKAGE_VERSION_PATCH ${LIBMINC_PACKAGE_VERSION_PATCH})
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Medical Imaging library core IO")

  OPTION(LIBMINC_BUILD_SHARED_LIBS   "Build libminc with shared libraries."        OFF)


  OPTION(LIBMINC_MINC1_SUPPORT           "Support minc1 file format, requires NETCDF" OFF)
  OPTION(LIBMINC_BUILD_EZMINC            "Build C++ EMINC library based on libminc1" OFF)
  OPTION(LIBMINC_BUILD_EZMINC_EXAMPLES   "Build EZminc examples" OFF)
  OPTION(LIBMINC_USE_NIFTI               "Build with NIfTI support" OFF)
  OPTION(LIBMINC_USE_SYSTEM_NIFTI        "Use system NIfTI-1 library" OFF)

  OPTION(LIBMINC_USE_ASAN                "Build with Address Sanitizer" OFF)

  SET (LIBMINC_EXPORTED_TARGETS "LIBMINC-targets")
  SET (LIBMINC_INSTALL_BIN_DIR bin)
  SET (LIBMINC_INSTALL_LIB_DIR lib${LIB_SUFFIX})
  SET (LIBMINC_INSTALL_INCLUDE_DIR include)
  SET (LIBMINC_INSTALL_DATA_DIR share)

  IF(LIBMINC_MINC1_SUPPORT)
    FIND_PACKAGE(NETCDF REQUIRED)
  ENDIF(LIBMINC_MINC1_SUPPORT)

  # external packages
  FIND_PACKAGE(ZLIB REQUIRED)
  SET(HDF5_NO_FIND_PACKAGE_CONFIG_FILE ON)
  FIND_PACKAGE(HDF5 REQUIRED COMPONENTS C )

  IF(LIBMINC_USE_NIFTI)
  IF (LIBMINC_USE_SYSTEM_NIFTI)
    FIND_PACKAGE(NIFTI)
  ELSE(LIBMINC_USE_SYSTEM_NIFTI)
    INCLUDE(ExternalProject)
    INCLUDE(BuildNIFTI)
    build_nifti(${CMAKE_INSTALL_PREFIX} ${CMAKE_BINARY_DIR}/external)
  ENDIF(LIBMINC_USE_SYSTEM_NIFTI)
  ELSE()
  SET(NIFTI_FOUND OFF)
  ENDIF()

  SET(HAVE_ZLIB ON)
ELSE(NOT LIBMINC_EXTERNALLY_CONFIGURED)
  #TODO: set paths for HDF5 etc
ENDIF(NOT LIBMINC_EXTERNALLY_CONFIGURED)

IF(NOT DEFINED CMAKE_MACOSX_RPATH)
  set(CMAKE_MACOSX_RPATH ON)
ENDIF(NOT DEFINED CMAKE_MACOSX_RPATH)

# Configure libminc
IF(LIBMINC_BUILD_SHARED_LIBS)
  SET(LIBRARY_TYPE SHARED)
  SET(LIBRARY_INSTALL LIBRARY)
ELSE(LIBMINC_BUILD_SHARED_LIBS)
  SET(LIBRARY_TYPE STATIC)
  SET(LIBRARY_INSTALL ARCHIVE)
ENDIF(LIBMINC_BUILD_SHARED_LIBS)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  SET(DEBUG "1")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

# add for building relocatable library
IF(UNIX)
  SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF(UNIX)


IF(LIBMINC_USE_ASAN)
  add_compile_options(-fno-omit-frame-pointer -fsanitize=address)
  add_link_options(-fno-omit-frame-pointer -fsanitize=address)
ENDIF(LIBMINC_USE_ASAN)


# check for prereqs
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckLibraryExists)

CHECK_FUNCTION_EXISTS(mkstemp  HAVE_MKSTEMP)
CHECK_FUNCTION_EXISTS(tmpnam   HAVE_TMPNAM)
CHECK_FUNCTION_EXISTS(tempnam  HAVE_TEMPNAM)
CHECK_FUNCTION_EXISTS(strerror HAVE_STRERROR)
CHECK_FUNCTION_EXISTS(popen    HAVE_POPEN)
CHECK_FUNCTION_EXISTS(fork     HAVE_WORKING_FORK)
CHECK_FUNCTION_EXISTS(vfork    HAVE_WORKING_VFORK)
CHECK_FUNCTION_EXISTS(fdopen   HAVE_FDOPEN)
CHECK_FUNCTION_EXISTS(strdup   HAVE_STRDUP)
CHECK_FUNCTION_EXISTS(getpwnam HAVE_GETPWNAM)
CHECK_FUNCTION_EXISTS(select   HAVE_SELECT)
CHECK_FUNCTION_EXISTS(strerror HAVE_STRERROR)
CHECK_FUNCTION_EXISTS(sysconf  HAVE_SYSCONF)
CHECK_FUNCTION_EXISTS(system   HAVE_SYSTEM)

if(NOT MSVC)
  set(CMAKE_REQUIRED_LIBRARIES m)
endif()

set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE=1 )

CHECK_SYMBOL_EXISTS(rint     "math.h"  HAVE_RINT)
CHECK_SYMBOL_EXISTS(copysign "math.h" HAVE_COPYSIGN)
CHECK_SYMBOL_EXISTS(round    "math.h" HAVE_ROUND)
CHECK_SYMBOL_EXISTS(fabsf    "math.h" HAVE_FABSF)
unset(CMAKE_REQUIRED_LIBRARIES)
unset(CMAKE_REQUIRED_DEFINITIONS)


CHECK_FUNCTION_EXISTS(srand48   HAVE_SRAND48)
CHECK_FUNCTION_EXISTS(drand48   HAVE_DRAND48)
CHECK_FUNCTION_EXISTS(sleep     HAVE_SLEEP)

CHECK_SYMBOL_EXISTS(gettimeofday "sys/time.h" HAVE_GETTIMEOFDAY)

CHECK_LIBRARY_EXISTS(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME_RT)

IF(HAVE_CLOCK_GETTIME_RT)
  SET(RT_LIBRARY "rt")
  SET(HAVE_CLOCK_GETTIME ON)
ELSE(HAVE_CLOCK_GETTIME_RT)
  # first check if the function is defined in header
  CHECK_SYMBOL_EXISTS(clock_gettime "time.h" HAVE_CLOCK_GETTIME_SYMBOL)
  # then check that it's available
  IF(HAVE_CLOCK_GETTIME_SYMBOL)
    TRY_RUN(HAVE_CLOCK_GETTIME_RUN HAVE_CLOCK_GETTIME_COMP
            ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/check_clock_gettime.c
            )
    IF(HAVE_CLOCK_GETTIME_RUN EQUAL 0 AND HAVE_CLOCK_GETTIME_COMP)
      SET(HAVE_CLOCK_GETTIME ON)
    ELSE()
      SET(HAVE_CLOCK_GETTIME OFF)
    ENDIF()
  ENDIF()


ENDIF(HAVE_CLOCK_GETTIME_RT)

INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES(float.h     HAVE_FLOAT_H)
CHECK_INCLUDE_FILES(sys/dir.h   HAVE_SYS_DIR_H)
CHECK_INCLUDE_FILES(sys/ndir.h  HAVE_SYS_NDIR_H)
CHECK_INCLUDE_FILES(sys/stat.h  HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES(sys/wait.h  HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILES(sys/time.h  HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES(time.h      HAVE_TIME_H)
CHECK_INCLUDE_FILES(values.h    HAVE_VALUES_H)
CHECK_INCLUDE_FILES(unistd.h    HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(dirent.h    HAVE_DIRENT_H)
CHECK_INCLUDE_FILES(memory.h    HAVE_MEMORY_H)
CHECK_INCLUDE_FILES(stdlib.h    HAVE_STDLIB_H)
CHECK_INCLUDE_FILES(fcntl.h     HAVE_FCNTL_H)
CHECK_INCLUDE_FILES(dlfcn.h     HAVE_DLFCN_H)
CHECK_INCLUDE_FILES(vfork.h     HAVE_VFORK_H)
CHECK_INCLUDE_FILES(inttypes.h  HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES(string.h    HAVE_STRING_H)
CHECK_INCLUDE_FILES(strings.h   HAVE_STRINGS_H)
CHECK_INCLUDE_FILES(pwd.h       HAVE_PWD_H)
CHECK_INCLUDE_FILES(sys/select.h    HAVE_SYS_SELECT_H)


ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DMINC2=1)
IF(NOT APPLE)
ADD_DEFINITIONS(-D_XOPEN_SOURCE=500)
ENDIF(NOT APPLE)
# aliases
SET(VERSION "${LIBMINC_EXTERNAL_LIB_PREFIX}${LIBMINC_PACKAGE_VERSION}")
SET(HAVE_MINC1 ${LIBMINC_MINC1_SUPPORT})
SET(HAVE_MINC2 ON)
SET(MINC2 "1")

IF(LIBMINC_MINC1_SUPPORT)
 ADD_DEFINITIONS(-DHAVE_MINC1=1)
ENDIF(LIBMINC_MINC1_SUPPORT)

IF(LIBMINC_BUILD_EZMINC)
  set(LIBMINC_INCLUDE_DIRS_CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/ezminc
  )
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/ezminc
   )
ENDIF(LIBMINC_BUILD_EZMINC)

IF(NIFTI_FOUND)
    SET(LIBMINC_NIFTI_SUPPORT ON)
    SET(LIBMINC_NIFTI_SOURCES
      volume_io/Volumes/input_nifti.c
      volume_io/Volumes/input_mgh.c
      volume_io/Volumes/input_nrrd.c
    )
ENDIF(NIFTI_FOUND)

# config files for build
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

# others
#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/epm-header.in ${CMAKE_CURRENT_BINARY_DIR}/epm-header)

# set the master INCLUDE directories
INCLUDE_DIRECTORIES(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/libcommon
   ${CMAKE_CURRENT_SOURCE_DIR}/libsrc2
   ${CMAKE_CURRENT_SOURCE_DIR}/libsrc
   ${CMAKE_CURRENT_SOURCE_DIR}/volume_io/Include
   ${HDF5_INCLUDE_DIRS}
   )

IF(LIBMINC_BUILD_EZMINC AND LIBMINC_MINC1_SUPPORT)
  INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/ezminc )
ENDIF()

IF(LIBMINC_NIFTI_SUPPORT)
    INCLUDE_DIRECTORIES(
        ${NIFTI_INCLUDE_DIR}
    )
ENDIF(LIBMINC_NIFTI_SUPPORT)

SET(minc_common_SRCS
  libcommon/minc2_error.c
  libcommon/minc_config.c
  libcommon/minc_error.c
  libcommon/ParseArgv.c
  libcommon/read_file_names.c
  libcommon/restructure.c
  libcommon/time_stamp.c
)

SET(minc_common_HEADERS
  libcommon/minc2_error.h
  libcommon/minc_config.h
  libcommon/minc_error.h
  libcommon/ParseArgv.h
  libcommon/read_file_names.h
  libcommon/restructure.h
  libcommon/time_stamp.h
  libcommon/minc_common_defs.h
  volume_io/Include/volume_io.h # VF: WTF?
)

SET(minc1_LIB_SRCS
   libsrc/dim_conversion.c
   libsrc/image_conversion.c
   libsrc/minc_convenience.c
   libsrc/nd_loop.c
   libsrc/netcdf_convenience.c
   libsrc/value_conversion.c
   libsrc/voxel_loop.c
   libsrc/hdf_convenience.c
   libsrc/minc_compat.c
   libsrc/minc_simple.c
   libsrc/minc_format_convert.c
   )

SET(minc1_HEADERS
  libsrc/minc.h
  libsrc/voxel_loop.h
  libsrc/nd_loop.h
  libsrc/minc_compat.h
  libsrc/minc_simple.h
  libsrc/minc_format_convert.h
)

SET(minc2_LIB_SRCS
   libsrc2/convert.c
   libsrc2/datatype.c
   libsrc2/dimension.c
   libsrc2/free.c
   libsrc2/grpattr.c
   libsrc2/hyper.c
   libsrc2/label.c
   libsrc2/m2util.c
   libsrc2/record.c
   libsrc2/slice.c
   libsrc2/valid.c
   libsrc2/volprops.c
   libsrc2/volume.c
   )

SET(minc2_HEADERS
  libsrc2/minc2.h
  libsrc2/minc2_defs.h
  libsrc2/minc2_structs.h
  libsrc2/minc2_api.h
)

# volume_io2
SET(volume_io_LIB_SRCS
   volume_io/Geometry/colour.c
   volume_io/Geometry/colour_def.c
   volume_io/Geometry/gaussian.c
   volume_io/Geometry/inverse.c
   volume_io/Geometry/newton.c
   volume_io/Geometry/points.c
   volume_io/Geometry/splines.c
   volume_io/Geometry/tensors.c
   volume_io/Geometry/transforms.c
   volume_io/MNI_formats/gen_xf_io.c
   volume_io/MNI_formats/gen_xfs.c
   volume_io/MNI_formats/grid_transforms.c
   volume_io/MNI_formats/mni_io.c
   volume_io/MNI_formats/tag_points.c
   volume_io/MNI_formats/thin_plate_spline.c
   volume_io/Prog_utils/alloc.c
   volume_io/Prog_utils/alloc_check.c
   volume_io/Prog_utils/arrays.c
   volume_io/Prog_utils/files.c
   volume_io/Prog_utils/print.c
   volume_io/Prog_utils/progress.c
   volume_io/Prog_utils/string.c
   volume_io/Prog_utils/time.c
   volume_io/Volumes/evaluate.c
   volume_io/Volumes/get_hyperslab.c
   volume_io/Volumes/input_free.c
   volume_io/Volumes/input_mnc.c
   volume_io/Volumes/input_volume.c
   volume_io/Volumes/multidim_arrays.c
   volume_io/Volumes/output_mnc.c
   volume_io/Volumes/output_volume.c
   volume_io/Volumes/set_hyperslab.c
   volume_io/Volumes/volume_cache.c
   volume_io/Volumes/volumes.c
   volume_io/Volumes/input_mnc2.c
   volume_io/Volumes/output_mnc2.c
)

IF(LIBMINC_NIFTI_SUPPORT)
    SET(volume_io_LIB_SRCS
        ${volume_io_LIB_SRCS}
        ${LIBMINC_NIFTI_SOURCES}
    )
ENDIF(LIBMINC_NIFTI_SUPPORT)


SET(volume_io_HEADERS
    volume_io/Include/volume_io/alloc.h
    volume_io/Include/volume_io/arrays.h
    volume_io/Include/volume_io/basic.h
    volume_io/Include/volume_io/def_math.h
    volume_io/Include/volume_io/files.h
    volume_io/Include/volume_io/geom_structs.h
    volume_io/Include/volume_io/geometry.h
    volume_io/Include/volume_io/multidim.h
    volume_io/Include/volume_io/progress.h
    volume_io/Include/volume_io/string_funcs.h
    volume_io/Include/volume_io/system_dependent.h
    volume_io/Include/volume_io/transforms.h
    volume_io/Include/volume_io/vol_io_prototypes.h
    volume_io/Include/volume_io/volume.h
    volume_io/Include/volume_io/volume_cache.h
)

SET(LIBMINC_LIBRARY   ${LIBMINC_EXTERNAL_LIB_PREFIX}minc2)

# a hack for APPLE
IF(LIBMINC_BUILD_SHARED_LIBS)
  SET(LIBMINC_LIBRARY_STATIC ${LIBMINC_EXTERNAL_LIB_PREFIX}minc2_static)
ELSE(LIBMINC_BUILD_SHARED_LIBS)
  SET(LIBMINC_LIBRARY_STATIC ${LIBMINC_LIBRARY})
ENDIF(LIBMINC_BUILD_SHARED_LIBS)

IF(LIBMINC_NIFTI_SUPPORT)
    SET(NIFTI_LIBRARIES
        ${NIFTI_LIBRARY} ${ZNZ_LIBRARY}
    )
    get_filename_component(NIFTI_LIBRARY_NAME "${NIFTI_LIBRARY}" NAME)
    get_filename_component(ZNZ_LIBRARY_NAME "${ZNZ_LIBRARY}" NAME)

ELSE(LIBMINC_NIFTI_SUPPORT)
    SET(NIFTI_LIBRARIES )
    SET(NIFTI_LIBRARY )
    SET(ZNZ_LIBRARY )
    SET(NIFTI_LIBRARY_NAME )
    SET(ZNZ_LIBRARY_NAME )
ENDIF(LIBMINC_NIFTI_SUPPORT)

IF(NOT HDF5_LIBRARY AND HDF5_LIBRARIES)
    SET(HDF5_LIBRARY ${HDF5_LIBRARIES})
ENDIF()

get_filename_component(HDF5_LIBRARY_NAME "${HDF5_LIBRARY}" NAME)
get_filename_component(ZLIB_LIBRARY_NAME "${ZLIB_LIBRARY}" NAME)

SET(LIBMINC_LIBRARIES        ${LIBMINC_LIBRARY} ${HDF5_LIBRARY} ${NIFTI_LIBRARIES} ${ZLIB_LIBRARY})
SET(LIBMINC_LIBRARIES_CONFIG ${LIBMINC_LIBRARY} ${HDF5_LIBRARY_NAME} ${NIFTI_LIBRARY_NAME} ${ZNZ_LIBRARY_NAME} ${ZLIB_LIBRARY_NAME})

SET(LIBMINC_STATIC_LIBRARIES ${LIBMINC_LIBRARY_STATIC} ${HDF5_LIBRARY} ${NIFTI_LIBRARIES} ${ZLIB_LIBRARY})
SET(LIBMINC_STATIC_LIBRARIES_CONFIG ${LIBMINC_LIBRARY_STATIC} ${HDF5_LIBRARY_NAME} ${NIFTI_LIBRARIES} ${ZLIB_LIBRARY_NAME})

IF(UNIX)
  SET(LIBMINC_LIBRARIES ${LIBMINC_LIBRARIES} m ${CMAKE_DL_LIBS} ${RT_LIBRARY})
  SET(LIBMINC_STATIC_LIBRARIES ${LIBMINC_STATIC_LIBRARIES} m ${CMAKE_DL_LIBS} ${RT_LIBRARY})

  SET(LIBMINC_LIBRARIES_CONFIG ${LIBMINC_LIBRARIES_CONFIG} m ${CMAKE_DL_LIBS} ${RT_LIBRARY_NAME})
  SET(LIBMINC_STATIC_LIBRARIES_CONFIG ${LIBMINC_STATIC_LIBRARIES_CONFIG} m ${CMAKE_DL_LIBS} ${RT_LIBRARY_NAME})
ENDIF(UNIX)

SET(minc_LIB_SRCS ${minc2_LIB_SRCS} ${minc_common_SRCS})
SET(minc_HEADERS  ${minc2_HEADERS} ${minc_common_HEADERS})

IF(LIBMINC_MINC1_SUPPORT)
  SET(minc_HEADERS  ${minc_HEADERS}  ${minc1_HEADERS})
  SET(minc_LIB_SRCS ${minc_LIB_SRCS} ${minc1_LIB_SRCS})
ENDIF(LIBMINC_MINC1_SUPPORT)


# Keep this variable for compatibility
SET(VOLUME_IO_LIBRARY  ${LIBMINC_EXTERNAL_LIB_PREFIX}minc2)
SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})

ADD_LIBRARY(${LIBMINC_LIBRARY} ${LIBRARY_TYPE} ${minc_LIB_SRCS} ${minc_HEADERS} ${volume_io_LIB_SRCS} ${volume_io_HEADERS} )

IF(NIFTI_FOUND AND NOT LIBMINC_USE_SYSTEM_NIFTI)
    ADD_DEPENDENCIES(${LIBMINC_LIBRARY} NIFTI)
ENDIF()


TARGET_LINK_LIBRARIES(${LIBMINC_LIBRARY} ${HDF5_LIBRARY} ${NIFTI_LIBRARIES} ${ZLIB_LIBRARY} ${RT_LIBRARY}) #

IF(LIBMINC_MINC1_SUPPORT)
  INCLUDE_DIRECTORIES(${NETCDF_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(${LIBMINC_LIBRARY} ${NETCDF_LIBRARY})
    get_filename_component(NETCDF_LIBRARY_NAME "${NETCDF_LIBRARY}" NAME)
ELSE(LIBMINC_MINC1_SUPPORT)
  SET(NETCDF_LIBRARY_NAME )
ENDIF(LIBMINC_MINC1_SUPPORT)

IF(NOT LIBMINC_USE_SYSTEM_NIFTI AND NOT LIBMINC_EXTERNALLY_CONFIGURED AND NIFTI_FOUND)
  ADD_DEPENDENCIES(minc2 NIFTI)
ENDIF()

#EXPORT(TARGETS ${LIBMINC_LIBRARY} FILE "${LIBMINC_EXPORTED_TARGETS}.cmake")

IF(UNIX)
  TARGET_LINK_LIBRARIES(${LIBMINC_LIBRARY} m ${CMAKE_DL_LIBS} )

#ITK: Skip static library build
IF(0)
  IF(LIBMINC_BUILD_SHARED_LIBS)
    ADD_LIBRARY(${LIBMINC_LIBRARY_STATIC} STATIC ${minc_LIB_SRCS} ${minc_HEADERS} ${volume_io_LIB_SRCS} ${volume_io_HEADERS} )
    TARGET_LINK_LIBRARIES(${LIBMINC_LIBRARY_STATIC} ${HDF5_LIBRARY} ${NIFTI_LIBRARIES} ${ZLIB_LIBRARY} ${RT_LIBRARY} m ${CMAKE_DL_LIBS} )
    IF(LIBMINC_MINC1_SUPPORT)
      TARGET_LINK_LIBRARIES(${LIBMINC_LIBRARY} ${NETCDF_LIBRARY})
    ENDIF(LIBMINC_MINC1_SUPPORT)
  ENDIF(LIBMINC_BUILD_SHARED_LIBS)
#ITK: Skip static library build
ENDIF(0)
ENDIF(UNIX)

SET_TARGET_PROPERTIES(${LIBMINC_LIBRARY}
 PROPERTIES
  SOVERSION ${LIBMINC_SOVERSION})

IF(LIBMINC_MINC1_SUPPORT)
  SET(LIBMINC_LIBRARIES        ${LIBMINC_LIBRARIES} ${NETCDF_LIBRARY} )
  SET(LIBMINC_STATIC_LIBRARIES ${LIBMINC_STATIC_LIBRARIES} ${NETCDF_LIBRARY} )

  SET(LIBMINC_LIBRARIES_CONFIG        ${LIBMINC_LIBRARIES_CONFIG} ${NETCDF_LIBRARY_NAME} )
  SET(LIBMINC_STATIC_LIBRARIES_CONFIG ${LIBMINC_STATIC_LIBRARIES_CONFIG} ${NETCDF_LIBRARY_NAME} )
ENDIF(LIBMINC_MINC1_SUPPORT)

IF( LIBMINC_INSTALL_LIB_DIR )
  INSTALL(
    TARGETS
      ${LIBMINC_LIBRARY}
    EXPORT
      ${LIBMINC_EXPORTED_TARGETS}
    LIBRARY DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
    ARCHIVE DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
    RUNTIME DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
  )
ENDIF( LIBMINC_INSTALL_LIB_DIR )

IF(LIBMINC_INSTALL_INCLUDE_DIR AND NOT LIBMINC_INSTALL_NO_DEVELOPMENT)
  INSTALL(FILES   ${minc_HEADERS}      DESTINATION  ${LIBMINC_INSTALL_INCLUDE_DIR} COMPONENT headers )
  INSTALL(FILES   ${volume_io_HEADERS} DESTINATION  ${LIBMINC_INSTALL_INCLUDE_DIR}/volume_io COMPONENT headers)
ENDIF(LIBMINC_INSTALL_INCLUDE_DIR AND NOT LIBMINC_INSTALL_NO_DEVELOPMENT)

IF(LIBMINC_BUILD_EZMINC AND LIBMINC_MINC1_SUPPORT)
  ADD_SUBDIRECTORY( ezminc )
  SET(EZMINC_LIBRARIES                   minc_io ${LIBMINC_LIBRARIES})
  SET(EZMINC_LIBRARIES_CONFIG            minc_io ${LIBMINC_LIBRARIES_CONFIG})
ELSE()
  SET(EZMINC_LIBRARIES                   "")
  SET(EZMINC_LIBRARIES_CONFIG            "")
ENDIF()


# config for install dir
SET(LIBMINC_USE_FILE_CONFIG     "\${LIBMINC_INSTALL_PREFIX}/lib/cmake/Use${LIBMINC_EXTERNAL_LIB_PREFIX}LIBMINC.cmake" )
SET(LIBMINC_INCLUDE_DIRS_CONFIG "\${LIBMINC_INSTALL_PREFIX}/include" )
SET(LIBMINC_LIBRARY_DIRS_CONFIG "\${LIBMINC_INSTALL_PREFIX}/lib" )
SET(LIBMINC_STATIC_LIBRARIES_CONFIG   "" )
SET(VOLUME_IO_LIBRARY_STATIC "")

configure_file(LIBMINCConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${LIBMINC_EXTERNAL_LIB_PREFIX}LIBMINCConfig.cmake @ONLY )

configure_file(UseLIBMINC.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Use${LIBMINC_EXTERNAL_LIB_PREFIX}LIBMINC.cmake @ONLY)

# configure for build dir
SET(LIBMINC_USE_FILE_CONFIG  ${CMAKE_CURRENT_BINARY_DIR}/UseLIBMINC.cmake)

SET(LIBMINC_INCLUDE_DIRS_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libcommon
  ${CMAKE_CURRENT_SOURCE_DIR}/libsrc2
  ${CMAKE_CURRENT_SOURCE_DIR}/volume_io/Include
)
IF(LIBMINC_MINC1_SUPPORT)
  SET(LIBMINC_INCLUDE_DIRS_CONFIG
    ${LIBMINC_INCLUDE_DIRS_CONFIG}
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc
  )

  IF(LIBMINC_BUILD_EZMINC)
    SET(LIBMINC_INCLUDE_DIRS_CONFIG ${LIBMINC_INCLUDE_DIRS_CONFIG}
      ${CMAKE_CURRENT_SOURCE_DIR}/ezminc )
  ENDIF()
ENDIF(LIBMINC_MINC1_SUPPORT)

SET(LIBMINC_LIBRARY_DIRS_CONFIG        ${CMAKE_CURRENT_BINARY_DIR})
SET(LIBMINC_LIBRARIES_CONFIG           ${LIBMINC_LIBRARIES})
SET(LIBMINC_STATIC_LIBRARIES_CONFIG    ${LIBMINC_STATIC_LIBRARIES})
SET(EZMINC_LIBRARIES_CONFIG            ${EZMINC_LIBRARIES})

IF(LIBMINC_MINC1_SUPPORT)
  SET(LIBMINC_LIBRARY_DIRS_CONFIG  ${LIBMINC_LIBRARY_DIRS_CONFIG} ${CMAKE_CURRENT_BINARY_DIR}/ezminc)
ENDIF(LIBMINC_MINC1_SUPPORT)

configure_file(LIBMINCConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/LIBMINCConfig.cmake @ONLY )

configure_file(UseLIBMINC.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/UseLIBMINC.cmake @ONLY)


IF(LIBMINC_INSTALL_LIB_DIR AND NOT LIBMINC_INSTALL_NO_DEVELOPMENT)
  INSTALL(
    FILES
     ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Use${LIBMINC_EXTERNAL_LIB_PREFIX}LIBMINC.cmake
     ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${LIBMINC_EXTERNAL_LIB_PREFIX}LIBMINCConfig.cmake
    DESTINATION
     ${LIBMINC_INSTALL_LIB_DIR}/cmake
    COMPONENT Development)
ENDIF(LIBMINC_INSTALL_LIB_DIR AND NOT LIBMINC_INSTALL_NO_DEVELOPMENT)

# testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(testdir)
ENDIF(BUILD_TESTING)

IF(NOT LIBMINC_EXTERNALLY_CONFIGURED)
  INCLUDE(CPack)
ENDIF(NOT LIBMINC_EXTERNALLY_CONFIGURED)
