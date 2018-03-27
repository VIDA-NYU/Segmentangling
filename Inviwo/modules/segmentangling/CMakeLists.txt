#--------------------------------------------------------------------
ivw_module(Segmentangling)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/contourfilter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datapreprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/loadcontourtree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/segmentationidraycaster.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/straightener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/tetmesher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumecollectiongenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeexportgenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesliceoverlay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/defer.h

    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/ContourTreeData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/DisjointSets.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/Grid3D.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/MergeTree.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/Persistence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/ScalarFunction.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/SimFunction.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/SimplifyCT.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/TopologicalFeatures.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/TriMesh.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/ContourTree.hpp
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/contourfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datapreprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/loadcontourtree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/segmentationidraycaster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/straightener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/tetmesher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumecollectiongenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeexportgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesliceoverlay.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/src/utils/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/src/utils/yixin_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/src/deformation_constraints.cpp


    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/ContourTree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/ContourTreeData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/Grid3D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/Hypervolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/MergeTree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/Persistence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/SimplifyCT.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/TopologicalFeatures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ContourTree/TriMesh.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/segmentationraycaster.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add definition

# ivw_add_dependencies_on_target(target ${${mod}_target} "ext/qhull/lib/qhullcpp.lib" "ext/qhull/lib/qhullstatic.lib" "ext/qhull/lib/qhullstatic_r.lib")

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# libigl
option(LIBIGL_USE_STATIC_LIBRARY "Use libigl as static library" ON)
option(LIBIGL_WITH_ANTTWEAKBAR       "Use AntTweakBar"    OFF)
option(LIBIGL_WITH_CGAL              "Use CGAL"           OFF)
option(LIBIGL_WITH_COMISO            "Use CoMiso"         OFF)
option(LIBIGL_WITH_CORK              "Use Cork"           OFF)
option(LIBIGL_WITH_EMBREE            "Use Embree"         OFF)
option(LIBIGL_WITH_LIM               "Use LIM"            OFF)
option(LIBIGL_WITH_MATLAB            "Use Matlab"         OFF)
option(LIBIGL_WITH_MOSEK             "Use MOSEK"          OFF)
option(LIBIGL_WITH_NANOGUI           "Use Nanogui menu"   OFF)
option(LIBIGL_WITH_OPENGL            "Use OpenGL"         OFF)
option(LIBIGL_WITH_OPENGL_GLFW_IMGUI "Use ImGui"          OFF)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"           OFF)
option(LIBIGL_WITH_PNG               "Use PNG"            OFF)
option(LIBIGL_WITH_PYTHON            "Use Python"         OFF)
option(LIBIGL_WITH_TETGEN            "Use Tetgen"         OFF)
option(LIBIGL_WITH_TRIANGLE          "Use Triangle"       OFF)
option(LIBIGL_WITH_VIEWER            "Use OpenGL viewer"  OFF)
option(LIBIGL_WITH_XML               "Use XML"            OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/cmake)
set(ENV{LIBIGLROOT} ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/external/libigl)
find_package(LIBIGL REQUIRED QUIET)

target_include_directories(inviwo-module-segmentangling PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/src")
target_include_directories(inviwo-module-segmentangling PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/src/tests
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../fish_deformation/external/libigl/
)
target_include_directories(inviwo-module-segmentangling PUBLIC
    "C:/ab7512/SuiteSparse/include"
    "C:/ab7512/SuiteSparse/CHOLMOD/include"
    "C:/ab7512/OpenBLAS-v0.2.19-Win64-int32/include"
)

# target_link_libraries(inviwo-module-segmentangling PUBLIC igl::core igl::opengl_glfw igl::opengl_glfw_imgui)
target_link_libraries(inviwo-module-segmentangling PUBLIC igl::core)

if (WIN32)
  target_link_libraries(inviwo-module-segmentangling PUBLIC
    "C:/ab7512/OpenBLAS-v0.2.19-Win64-int32/lib/libopenblas.dll.a"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libcholmodd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/suitesparseconfigd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/metisd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libamdd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libbtfd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libcamdd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libccolamdd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libcolamdd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libcxsparsed.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libklud.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libldld.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libspqrd.lib"
    debug "C:/ab7512/SuiteSparse/lib/Debug/x64/libumfpackd.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libcholmod.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/suitesparseconfig.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/metis.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libamd.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libbtf.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libcamd.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libccolamd.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libcolamd.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libcxsparse.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libklu.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libldl.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libspqr.lib"
    optimized "C:/ab7512/SuiteSparse/lib/Release/x64/libumfpack.lib"
  )

else ()
  target_link_libraries(inviwo-module-segmentangling PUBLIC cholmod)
endif ()

target_compile_definitions(inviwo-module-segmentangling PUBLIC "__CL_ENABLE_EXCEPTIONS")

find_package(Qt5Core)
target_link_libraries(inviwo-module-segmentangling PUBLIC Qt5::Core)

target_link_libraries(
    ${${mod}_target}
    PUBLIC
    debug "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullcpp_d.lib"
    debug "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullstatic_rd.lib"
    optimized "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullcpp.lib"
    optimized "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullstatic_r.lib"

    
    # debug "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullcpp_d.lib"
    # debug "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhull_d.lib"
    # debug "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhull_rd.lib"
    # debug "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhull_pd.lib"
    # debug "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullstatic_d.lib"
    # debug "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullstatic_rd.lib"
    # optimized "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullcpp.lib"
    # optimized "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhull.lib"
    # optimized "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhull_r.lib"
    # optimized "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhull_p.lib"
    # optimized "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullstatic.lib"
    # optimized "${CMAKE_CURRENT_SOURCE_DIR}/ext/qhull/lib/qhullstatic_r.lib"
)

#--------------------------------------------------------------------
# Handle external resources
if(IVW_USE_GENERATED_RESOURCES)
#--------------------------------------------------------------------
# Generate shader resource (if GENERATE_SHADER_RESOURCES is checked)
ivw_generate_shader_resource(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})
else()
#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
endif()