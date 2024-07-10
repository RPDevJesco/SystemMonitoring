cmake_minimum_required(VERSION 3.28)
project(SystemInformationGatherer)

set(CMAKE_CXX_STANDARD 26)

# Change STATIC to SHARED to create a DLL
add_library(SystemInfoGatherer SHARED SystemInfo.cpp SystemInfo.hpp)

if(WIN32)
    target_link_libraries(SystemInfoGatherer PRIVATE iphlpapi wbemuuid dxgi)
    # Add this line to define DLL_EXPORTS when building the DLL
    target_compile_definitions(SystemInfoGatherer PRIVATE DLL_EXPORTS)
elseif(APPLE)
    # Mac-specific settings if needed
elseif(UNIX)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(SystemInfoGatherer PRIVATE ${X11_LIBRARIES} ${OPENGL_LIBRARIES})
endif()

# Optionally, you can add an executable to test the library
#add_executable(SystemInfoTest main.cpp)
#target_link_libraries(SystemInfoTest PRIVATE SystemInfoGatherer)