cmake_minimum_required(VERSION 3.16)
project(NcorrGUI CXX)

# --- Basic Qt Setup ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# --- Define Project Paths ---
set(RELEASE_ROOT_PATH "C:/Users/Ali/Pictures/release")
string(REPLACE "\\" "/" RELEASE_ROOT_PATH ${RELEASE_ROOT_PATH})

# --- Source Files ---
add_executable(NcorrGUI WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    dicparametersdialog.cpp
    dicparametersdialog.h
    dicparametersdialog.ui

    # Ncorr source files
    "${RELEASE_ROOT_PATH}/Ncorr_vs2015/Ncorr_vs2015/Array2D.cpp"
    "${RELEASE_ROOT_PATH}/Ncorr_vs2015/Ncorr_vs2015/Data2D.cpp"
    "${RELEASE_ROOT_PATH}/Ncorr_vs2015/Ncorr_vs2015/Disp2D.cpp"
    "${RELEASE_ROOT_PATH}/Ncorr_vs2015/Ncorr_vs2015/Image2D.cpp"
    "${RELEASE_ROOT_PATH}/Ncorr_vs2015/Ncorr_vs2015/ncorr.cpp"
    "${RELEASE_ROOT_PATH}/Ncorr_vs2015/Ncorr_vs2015/ROI2D.cpp"
    "${RELEASE_ROOT_PATH}/Ncorr_vs2015/Ncorr_vs2015/Strain2D.cpp"
)

# --- Include Directories ---
target_include_directories(NcorrGUI PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    "${RELEASE_ROOT_PATH}/Ncorr_vs2015/Ncorr_vs2015"
    "${RELEASE_ROOT_PATH}/external/release64/include"
    "${RELEASE_ROOT_PATH}/external/release64/include/suitesparse"
)

# --- Library Directories ---
target_link_directories(NcorrGUI PRIVATE "${RELEASE_ROOT_PATH}/external/release64/lib")

# --- Linking Libraries ---
target_link_libraries(NcorrGUI PRIVATE
    Qt5::Widgets
    opencv_core300 opencv_highgui300 opencv_imgcodecs300 opencv_imgproc300 opencv_videoio300
    libfftw3-3
    libspqr libcholmod libccolamd libcamd libcolamd libamd suitesparseconfig
    libbtf libklu libldl libumfpack metis
    liblapack libblas
)

# --- Post-Build Step: Copy DLLs ---
add_custom_command(TARGET NcorrGUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RELEASE_ROOT_PATH}/external/release64/dll"
    $<TARGET_FILE_DIR:NcorrGUI>
)

# --- Suppress specific MSVC warnings ---
if(MSVC)
    target_compile_options(NcorrGUI PRIVATE /wd4267) # Suppress size_t to int conversion warning
endif()
