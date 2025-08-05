cmake_minimum_required(VERSION 3.16)
project(NcorrGUI CXX)

# --- Basic Qt Setup ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# --- Define Project Paths using relative paths ---
set(NCORR_DIR "${CMAKE_SOURCE_DIR}/Ncorr")
set(EXTERNAL_BASE_PATH "${CMAKE_SOURCE_DIR}/external/release64")

# --- Source Files ---
add_executable(NcorrGUI WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    dicparametersdialog.cpp
    dicparametersdialog.h
    dicparametersdialog.ui

    # Ncorr source files from the project subdirectory
    "${NCORR_DIR}/Array2D.cpp"
    "${NCORR_DIR}/Data2D.cpp"
    "${NCORR_DIR}/Disp2D.cpp"
    "${NCORR_DIR}/Image2D.cpp"
    "${NCORR_DIR}/ncorr.cpp"
    "${NCORR_DIR}/ROI2D.cpp"
    "${NCORR_DIR}/Strain2D.cpp"
)

# --- Include Directories ---
target_include_directories(NcorrGUI PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    # Ncorr headers
    "${NCORR_DIR}"
    # External libraries headers
    "${EXTERNAL_BASE_PATH}/include"
    "${EXTERNAL_BASE_PATH}/include/suitesparse"
    "${EXTERNAL_BASE_PATH}/include/opencv2"
)

# --- Library Directories ---
target_link_directories(NcorrGUI PRIVATE "${EXTERNAL_BASE_PATH}/lib")

# --- Linking Libraries ---
target_link_libraries(NcorrGUI PRIVATE
    Qt5::Widgets
    opencv_core300
    opencv_highgui300
    opencv_imgcodecs300
    opencv_imgproc300
    opencv_videoio300
    libfftw3-3
    libspqr
    libcholmod
    libccolamd
    libcamd
    libcolamd
    libamd
    suitesparseconfig
    libbtf
    libklu
    libldl
    libumfpack
    metis
    liblapack
    libblas
)

# --- Post-Build Step: Copy DLLs to the build directory ---
add_custom_command(TARGET NcorrGUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${EXTERNAL_BASE_PATH}/dll"
    $<TARGET_FILE_DIR:NcorrGUI>
)

# --- Suppress specific MSVC warnings ---
if(MSVC)
    target_compile_options(NcorrGUI PRIVATE /wd4267 /wd4244)
endif()
