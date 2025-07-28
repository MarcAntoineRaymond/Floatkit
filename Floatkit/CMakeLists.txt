cmake_minimum_required(VERSION 3.15)
project(Floatkit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "*.h")

add_executable(Floatkit WIN32 ${SOURCES} ${HEADERS} Floatkit.rc)

# Link GDI+ (needed for GDI+ apps)
target_link_libraries(Floatkit gdiplus)