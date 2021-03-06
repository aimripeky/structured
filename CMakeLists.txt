#
#
#
# Dependencies:
#  * OpenCV
#  * OpenGL
# * GLEW
# * TCL/TK
# * vips
# *glib2
PROJECT(threadedStereo)
SET(THREADED_STEREO_LIB "ts")

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# This can removed if the required cmake version is increased to 2.6
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
SET(USE_AUV_FOR_STEREO OFF CACHE BOOL "Use AUVLibs for stereo if want to use AUV tree code")

#-----------------------#
# Includes              #
#-----------------------#
INCLUDE( ${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake )

find_package(PkgConfig)
find_package(OpenMP)
include(ExternalProject)
FIND_PACKAGE( OpenCV REQUIRED )
pkg_check_modules(GOBJ REQUIRED gobject-2.0)
pkg_check_modules(GLIB2 REQUIRED  glib-2.0)
set(OSG_COMPONENTS osg osgUtil osgViewer osgGA osgManipulator osgText osgDB)
find_package(OpenSceneGraph REQUIRED COMPONENTS ${OSG_COMPONENTS})
#GDAL
#INCLUDE( ${PROJECT_SOURCE_DIR}/VirtualPlanetBuilder/CMakeModules/FindGDAL.cmake )
find_package(GDAL REQUIRED)
include ("${CMAKE_SOURCE_DIR}/OptimizeForArchitecture.cmake")
include ("${CMAKE_SOURCE_DIR}/FindPythonModule.cmake")
pkg_check_modules(EIGEN3 REQUIRED eigen3)

#--------------------------------#
# Enforce an out-of-source build #
#--------------------------------#

STRING(COMPARE EQUAL "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" INSOURCE)
IF(INSOURCE)
  MESSAGE(FATAL_ERROR "This project requires an out of source build. This is handled by the Makefile. Please type 'make'.")
ENDIF(INSOURCE)


#--------------------------------#
# Build Options                  #
#--------------------------------#

# Options can be set from the command line when running cmake.
#   eg. cmake . -DCMAKE_BUILD_TYPE=Release -DDEBUG_PROFILING=on
    

# Build types are: Release, Debug, RelWithDebInfo, MinSizeRel
IF( NOT CMAKE_BUILD_TYPE )
 SET( CMAKE_BUILD_TYPE Release )  
ENDIF( NOT CMAKE_BUILD_TYPE )


#-----------------------#
# Installation Options  #
#-----------------------#

IF( NOT CMAKE_INSTALL_PREFIX )
   SET( CMAKE_INSTALL_PREFIX "/usr/local" )
ENDIF( NOT CMAKE_INSTALL_PREFIX )

IF(USE_AUV_FOR_STEREO)
message("Using AUV Code for Stereo")
    ADD_DEFINITIONS("-DUSE_AUV_LIBS=1")
IF( NOT LIBKEYPOINT_PREFIX )
   SET( LIBKEYPOINT_PREFIX /usr/local/ )
ENDIF( NOT LIBKEYPOINT_PREFIX )

# libpolyp
IF( NOT LIBSNAPPER_PREFIX )
   SET( LIBSNAPPER_PREFIX /usr/local/ )
ENDIF( NOT LIBSNAPPER_PREFIX )

IF( NOT LIBCARP_PREFIX )
   SET( LIBCARP_PREFIX /usr/local/ )
ENDIF( NOT LIBCARP_PREFIX )

#IF( NOT LIBPOLYP_PREFIX )
#   SET( LIBPOLYP_PREFIX /usr/local )
#ENDIF( NOT LIBPOLYP_PREFIX )
IF( NOT LIBPLANKTON_PREFIX )
   SET( LIBPLANKTON_PREFIX /usr/local/ )
ENDIF( NOT LIBPLANKTON_PREFIX )

#SET( LIBPOLYP_INC ${LIBPOLYP_PREFIX}/include/libpolyp )
#SET( LIBPOLYP_LIBRARY polyp )

# libcarp - FIXME: Remove use of this library
SET( LIBCARP_INC ${LIBCARP_PREFIX}/include/libcarp )
SET( LIBCARP_LIBRARY carp )
#pkg_check_modules( GTS "gts" )
#IF( NOT GTS_FOUND )
#   MESSAGE( FATAL_ERROR  "ERROR - Can't find Gts" )
#ENDIF( NOT GTS_FOUND )
#ADD_DEFINITIONS( ${GTS_CFLAGS} )
# libsnapper
SET( LIBSNAPPER_INC ${LIBSNAPPER_PREFIX}/include/libsnapper )
SET( LIBSNAPPER_LIBRARY snapper )
# libplankton
SET( LIBPLANKTON_INCLUDE_DIR ${LIBPLANKTON_PREFIX}/include/libplankton )
SET( LIBPLANKTON_LIB plankton )
CHECK_INCLUDE_FILE_CXX( "auv_matrix.hpp" HAVE_LIBPLANKTON "-I ${LIBPLANKTON_INCLUDE_DIR}" )
IF( NOT HAVE_LIBPLANKTON )
   MESSAGE( FATAL_ERROR  "ERROR - Can't find libplankton" )
ENDIF( NOT HAVE_LIBPLANKTON )

IF( NOT LIBKEYPOINT_PREFIX )
   SET( LIBKEYPOINT_PREFIX /usr/local/ )
ENDIF( NOT LIBKEYPOINT_PREFIX )
   SET( LIBKEYPOINT_INCLUDE_DIR ${LIBKEYPOINT_PREFIX}/include/libkeypoint )
   SET( LIBKEYPOINT_LIB keypoint  )
   ADD_DEFINITIONS( "-DHAVE_LIBKEYPOINT" )

   SET( CMAKE_REQUIRED_FLAGS "-I ${OPENCV_INCLUDE_DIRS} ${OPENCV_LIBRARIES}" )
   CHECK_INCLUDE_FILE_CXX( "keypoint.hpp" HAVE_LIBKEYPOINT "-I ${LIBKEYPOINT_INCLUDE_DIR}" )
   IF( NOT HAVE_LIBKEYPOINT )
#      MESSAGE( FATAL_ERROR  "ERROR - Can't find libkeypoint" )
   ENDIF( NOT HAVE_LIBKEYPOINT )
IF( NOT LIBADT_PREFIX )
   SET( LIBADT_PREFIX /usr/local )
ENDIF( NOT LIBADT_PREFIX )

# AUV data library
SET( LIBADT_INCLUDE_DIR ${LIBADT_PREFIX}/include/libadt )
SET( LIBADT_LIBRARY adt )
ENDIF(USE_AUV_FOR_STEREO)
#ExternalProject_Add(vrip
#  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vrip"
#  BUILD_IN_SOURCE 1
#  BUILD_COMMAND $(MAKE)
#  DOWNLOAD_COMMAND ""
#  UPDATE_COMMAND ""
#  CONFIGURE_COMMAND ""
#  INSTALL_COMMAND ""
#)

#-----------------------#
# Dependencies          #
#-----------------------#


# OpenGL and GLU
FIND_PACKAGE( OpenGL )
IF( NOT OPENGL_FOUND )
   MESSAGE( FATAL_ERROR  "ERROR - Can't find OpenGL" )
ENDIF( NOT OPENGL_FOUND )
IF( NOT OPENGL_GLU_FOUND )
   MESSAGE( FATAL_ERROR  "ERROR - Can't find GLU" )
ENDIF( NOT OPENGL_GLU_FOUND )


IF( NOT OSG_FOUND )
  MESSAGE( FATAL_ERROR  "ERROR - Can't find OSG" )
ENDIF( NOT OSG_FOUND )

pkg_check_modules( VIPS vips)
# UF
SET( UF_LIBRARY ufGeographicConversions ufGPSParser )

#
# find OpenMP
#

find_package( OpenMP )

IF (OPENMP_FOUND)
    MESSAGE(STATUS "found OpenMP, compiling with flags: " ${OpenMP_CXX_FLAGS} )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    MESSAGE(STATUS "flags are now: " ${CMAKE_CXX_FLAGS})
ENDIF(OPENMP_FOUND)
include(FindPythonInterp)
find_python_module(gdal REQUIRED)
find_python_module(osr REQUIRED)
find_python_module(ogr REQUIRED)

IF(NOT PY_GDAL)
 MESSAGE(FATAL_ERROR "Unable to find python bidning for GDAL: Try installing python-gdal")
ENDIF(NOT PY_GDAL)


IF(NOT PY_OSR)
 MESSAGE(FATAL_ERROR "Unable to find python bidning for OSR: Try installing python-gdal")
ENDIF(NOT PY_OSR)


IF(NOT PY_OGR)
 MESSAGE(FATAL_ERROR "Unable to find python bidning for OGR: Try installing python-gdal")
ENDIF(NOT PY_OGR)

#-----------------------#
# Compiler setup        #
#-----------------------#
ADD_DEFINITIONS( "-Wall -DGLIB_DISABLE_DEPRECATION_WARNINGS" )
cmake_policy(SET CMP0005 OLD)
#spatial lib defs
add_definitions("-DHAVE_FCNTL_H=1 -DHAVE_UNISTD_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_PTHREAD_H=1 -DHAVE_SYS_RESOURCE_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_STDINT_H=1 -DHAVE_GETTIMEOFDAY=1 -DHAVE_BZERO=1 -DHAVE_MEMSET=1 -DHAVE_MEMCPY=1 -DHAVE_BCOPY=1")
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/include
                     ${PROJECT_SOURCE_DIR}/vcgapps/
                     ${OPENCV_INCLUDE_DIRS}
		     ${OPENGL_INCLUDE_DIR}
                     ${VIPS_INCLUDE_DIRS}
                     trimesh2/
		     .
                     ann/include/
                    spatialindex-src-1.6.1/src spatialindex-src-1.6.1/include
                     "/opt/local/include"
"VirtualPlanetBuilder/include"
 ${OpenCV_INCLUDE_DIR}
                     ${GDAL_INCLUDE_DIR}
                     ${EIGEN3_INCLUDE_DIRS}
		     ${OSG_INCLUDE_DIR}
                   ${GLIB2_INCLUDE_DIRS}
                     ${LIBPOLYP_INC}
                     gts-0.7.6/src/
                     ${LIBSNAPPER_INC}
                     ${LIBCARP_INC}
                     ${LIBPLANKTON_INCLUDE_DIR}
                     ${LIBKEYPOINT_INCLUDE_DIR}
${LIBADT_INCLUDE_DIR}

)

LINK_DIRECTORIES( ${OPENCV_LIBRARY_DIRS}
${OpenCV_DIRS}
                  ${UF_LIBRARY_DIRS}
		          ${PROJECT_SOURCE_DIR}
${GLIB2_LIBRARY_DIRS}
                )
add_definitions(${GLIB2_CFLAGS})
LINK_LIBRARIES(
               ${OSG_LIBRARY}
 ${OSGDB_LIBRARY}
               ${OSGUTIL_LIBRARY}
     ${OSGTERRAIN_LIBRARY_DEBUG}
${GDAL_LIBRARY}
osgTerrain
osgFX
osgViewer
osgSim
               trimesh2
        ${OpenCV_LIBS}
                ${OSGSHADOW_LIBRARY}
                ${OSGTEXT_LIBRARY_DEBUG}
                ${OSGGA_LIBRARY_DEBUG}
                ${OPENTHREADS_LIBRARY}
         ${GOBJ_LIBRARIES}
         ${GLIB2_LIBRARIES}
         ${CMAKE_DL_LIBS}
          ${OPENGL_LIBRARY}
                ${LIBSNAPPER_LIBRARY}
                ${LIBCARP_LIBRARY}
                ${LIBPLANKTON_LIB}
                gts)


#-----------------------#
# Print Some Info       #
#-----------------------#

MESSAGE( "" )
MESSAGE( "CONFIGURATION SUMMARY" )
MESSAGE( "---------------------" )
MESSAGE( "" )
MESSAGE( "CMAKE_BUILD_TYPE = '${CMAKE_BUILD_TYPE}'" )
MESSAGE( "" )
MESSAGE( "CMAKE_INSTALL_PREFIX = '${CMAKE_INSTALL_PREFIX}'" )
MESSAGE( "" )

IF( CMAKE_BUILD_TYPE STREQUAL Debug )
   MESSAGE( "CMAKE_CXX_FLAGS_DEBUG = '${CMAKE_CXX_FLAGS_DEBUG}'" )
ENDIF( CMAKE_BUILD_TYPE STREQUAL Debug )

IF( CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo )
   MESSAGE( "CMAKE_CXX_FLAGS_RELWITHDEBINFO = '${CMAKE_CXX_FLAGS_RELWITHDEBINFO}'" )
ENDIF( CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo )

IF( CMAKE_BUILD_TYPE STREQUAL Release )
   MESSAGE( "CMAKE_CXX_FLAGS_RELEASE = '${CMAKE_CXX_FLAGS_RELEASE}'" )
ENDIF( CMAKE_BUILD_TYPE STREQUAL Release )

IF( CMAKE_BUILD_TYPE STREQUAL MinSizeRel )
   MESSAGE( "CMAKE_CXX_FLAGS_MINSIZEREL = '${CMAKE_CXX_FLAGS_MINSIZEREL}'" )
ENDIF( CMAKE_BUILD_TYPE STREQUAL MinSizeRel )

MESSAGE( "" )

#-----------------------#
# Build Instructions    #
#-----------------------#

# Locations for the library and binary files
SET( LIBRARY_OUTPUT_PATH     ${PROJECT_SOURCE_DIR}/lib )
SET( EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/ )
#needed to supress rdynamic which causes segfault
####
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
###

IF (CMAKE_BUILD_TYPE STREQUAL "Release")

OptimizeForArchitecture()
#add_definitions(-Wall -O3 -DNDEBUG -pipe -ffast-math -funroll-loops -ftree-vectorize -fomit-frame-pointer -pipe -mfpmath=sse -mmmx -msse -mtune=core2 -march=core2 -msse2 -msse3 -mssse3 -msse4)
ENDIF()
#SUBDIRS(src include tools )


#SUBDIRS(gts-0.7.6 trimesh2 vrip VirtualPlanetBuilder vcgapps spatialindex-src-1.6.1/src swrender triangulation ann/src/ CMVS-PMVS/program SiftGPU pba vsfm re_vision )
SUBDIRS(gts-0.7.6 trimesh2 vrip VirtualPlanetBuilder vcgapps spatialindex-src-1.6.1/src swrender triangulation ann/src/ CMVS-PMVS/program SiftGPU pba vsfm ts-sonar)

SET( SRCS
                Extents.cpp
calibFile.cpp
StereoEngine.cpp
plyfile.c
TexturedSource.cpp
Clipper.cpp
VPBInterface.cpp
PLYWriterNodeVisitor.cc
vertexData.cpp
TexPyrAtlas.cpp
Semaphore.cpp
TexturingQuery.cpp
TexturingQueryDidson.cpp
configFile.cpp
SeaBedIO.cpp
SpatialReference.cpp
MemUtils.cpp
CheckThreadPool.cpp
vcgapps/GenParam.cpp
                stereo_cells.cpp
OctreeBuilder.cpp
SplitBounds.cpp
ProgressBar.cpp
BuildAtlas.cpp
TightFit.cpp
GLImaging.cpp
mipmap.cpp
mesh_proc.cpp
stereo_dense.cpp
image_rect.cpp
   ANNWrapper.cpp             
)
ADD_DEFINITIONS( "-g")
ADD_LIBRARY( ${THREADED_STEREO_LIB} STATIC ${SRCS} ${SPINDEX_SRC})
TARGET_LINK_LIBRARIES (${THREADED_STEREO_LIB} ${THREADED_STEREO_LIB}-sonar)
ADD_EXECUTABLE( ${CMAKE_PROJECT_NAME} threadedStereo.cpp ShellCmd.cpp )
#ADD_EXECUTABLE( engine engine.cpp ShellCmd.cpp )

ADD_EXECUTABLE( stereo_mesh_gen stereo_mesh_gen.cpp )
ADD_EXECUTABLE(  genRangeImg genRangeImg.cpp )
ADD_EXECUTABLE(  im_shrink_noblack im_shrink_noblack.cpp)

ADD_EXECUTABLE( osgdem  VirtualPlanetBuilder/applications/osgdem/osgdem.cpp )
ADD_EXECUTABLE(create_mipmap create_mipmap.cpp)
ADD_EXECUTABLE( treeBBClip treeBBClip.cpp  )
ADD_EXECUTABLE( calcTexCoord calcTexCoord.cpp )
ADD_EXECUTABLE( calcTexCoordBundler calcTexCoordBundler.cpp )
ADD_EXECUTABLE( calcTexCoordDidson calcTexCoordDidson.cpp )

ADD_EXECUTABLE( testImageModel testImageModel.cpp )
ADD_EXECUTABLE( dicedImage dicedImage.cpp )
ADD_EXECUTABLE(deepzoomVIPS DeepZoom.cpp)
ADD_EXECUTABLE(singleImageTex singleImageTex.cpp)
ADD_EXECUTABLE(createSem createSem.cpp)
ADD_EXECUTABLE(vertCheck vertCheck.cpp)
ADD_EXECUTABLE(sparseJoin sparseJoin.cpp)
ADD_EXECUTABLE(vipsVTAtlas vipsVTAtlas.cpp)
ADD_EXECUTABLE(atlasmesh atlasmesh.cpp)
ADD_EXECUTABLE( borderClip borderClip.cpp  )
ADD_EXECUTABLE( dumpPCD dumpPCD.cpp  )
#ADD_EXECUTABLE( keypoint_mesh keypoint_mesh.cpp  )

ADD_EXECUTABLE(genOverlappingPoses genOverlappingPoses.cpp)

#ADD_EXECUTABLE(temp temp.cpp)
TARGET_LINK_LIBRARIES( genOverlappingPoses ${THREADED_STEREO_LIB} )
SET(OGF_LIBS basic math  cells  numeric_stuff image z  m )
LINK_LIBRARIES(osgTerrain  osgFX osg  spindex vips vipsCC)
TARGET_LINK_LIBRARIES( ${CMAKE_PROJECT_NAME} ${THREADED_STEREO_LIB}  vips vipsCC vpb spindex  ${THREADED_STEREO_LIB}-sonar ${OGF_LIBS})
#TARGET_LINK_LIBRARIES( engine ${THREADED_STEREO_LIB}  vips vipsCC vpb spindex)
TARGET_LINK_LIBRARIES( osgdem vpb  )
TARGET_LINK_LIBRARIES( vipsVTAtlas ${THREADED_STEREO_LIB}  vips vipsCC spindex   ${OGF_LIBS})
TARGET_LINK_LIBRARIES( atlasmesh ${THREADED_STEREO_LIB}  vips vipsCC spindex  ${OGF_LIBS})
TARGET_LINK_LIBRARIES(im_shrink_noblack ${THREADED_STEREO_LIB} vips vipsCC ${OGF_LIBS})
TARGET_LINK_LIBRARIES(create_mipmap ${THREADED_STEREO_LIB} vips vipsCC ${OGF_LIBS} dl)
#TARGET_LINK_LIBRARIES(keypoint_mesh ${THREADED_STEREO_LIB} vips vipsCC ${OGF_LIBS} dl)

TARGET_LINK_LIBRARIES( testImageModel ${THREADED_STEREO_LIB}  )
TARGET_LINK_LIBRARIES( deepzoomVIPS  vips vipsCC)
TARGET_LINK_LIBRARIES( testImageModel ${THREADED_STEREO_LIB}  )

TARGET_LINK_LIBRARIES( stereo_mesh_gen ${THREADED_STEREO_LIB}  )
TARGET_LINK_LIBRARIES( treeBBClip ${THREADED_STEREO_LIB} spindex  )
TARGET_LINK_LIBRARIES( genRangeImg ${THREADED_STEREO_LIB} spindex )

TARGET_LINK_LIBRARIES( vertCheck ${THREADED_STEREO_LIB} spindex )
TARGET_LINK_LIBRARIES( calcTexCoord ${THREADED_STEREO_LIB} spindex vpb vips vipsCC  ${OGF_LIBS})
TARGET_LINK_LIBRARIES( calcTexCoordBundler ${THREADED_STEREO_LIB} spindex vpb vips vipsCC  ${OGF_LIBS} ANN)
TARGET_LINK_LIBRARIES( calcTexCoordDidson ${THREADED_STEREO_LIB} spindex vpb vips vipsCC  ${OGF_LIBS})

TARGET_LINK_LIBRARIES( singleImageTex ${THREADED_STEREO_LIB} spindex vpb vips vipsCC  ${OGF_LIBS})
TARGET_LINK_LIBRARIES( borderClip ${THREADED_STEREO_LIB} spindex )
TARGET_LINK_LIBRARIES( dumpPCD vips vipsCC ${THREADED_STEREO_LIB} z)

TARGET_LINK_LIBRARIES( dicedImage vips vipsCC ${THREADED_STEREO_LIB} z)
TARGET_LINK_LIBRARIES( sparseJoin vips vipsCC ${THREADED_STEREO_LIB} z)

#TARGET_LINK_LIBRARIES( threshold ${THREADED_STEREO_LIB} )

# Disabled by DJLM. 
# Threaded stereo expects access to a large bunch of other executables
# and scripts and is expecting to in the same directory as these other
# tools. To do this  properly you need to know all of the external utilities
# that T.S. calls and these all need to appear in /usr/local/bin
# INSTALL_TARGETS( /bin ${CMAKE_PROJECT_NAME} )
