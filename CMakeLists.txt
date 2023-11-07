cmake_minimum_required(VERSION 3.10)
project(drmfb32)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(BSD REQUIRED libbsd)
pkg_check_modules(DRM REQUIRED libdrm)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

#--------------------------------------------------------------------------

add_library(drmfb32 STATIC libdrmfb32/drmMode.cxx
                           libdrmfb32/fileDescriptor.cxx
                           libdrmfb32/framebuffer8880.cxx
                           libdrmfb32/image8880.cxx
                           libdrmfb32/image8880Font.cxx
                           libdrmfb32/image8880Graphics.cxx
                           libdrmfb32/joystick.cxx
                           libdrmfb32/rgb8880.cxx)

include_directories(${PROJECT_SOURCE_DIR}/libdrmfb32
                    ${PROJECT_SOURCE_DIR}/thread-pool/include)
target_include_directories(drmfb32 PUBLIC ${DRM_INCLUDE_DIRS})
target_compile_options(drmfb32 PUBLIC ${DRM_CFLAGS_OTHER})
target_link_libraries(drmfb32 ${DRM_LIBRARIES})

set(EXTRA_LIBS ${EXTRA_LIBS} drmfb32)

#--------------------------------------------------------------------------

add_executable(info info/info.cxx
                    info/cpuTrace.cxx
                    info/dynamicInfo.cxx
                    info/memoryTrace.cxx
                    info/networkTrace.cxx
                    info/panel.cxx
                    info/system.cxx
                    info/temperatureTrace.cxx
                    info/trace.cxx
                    info/traceGraph.cxx
                    info/traceStack.cxx)

target_link_libraries(info drmfb32 ${BSD_LIBRARIES} ${DRM_LIBRARIES})
target_include_directories(info PUBLIC ${BSD_INCLUDE_DIRS})
target_compile_options(info PUBLIC ${BSD_CFLAGS_OTHER})

set_property(TARGET info PROPERTY SKIP_BUILD_RPATH TRUE)
install (TARGETS info RUNTIME DESTINATION bin)

#--------------------------------------------------------------------------

add_executable(boxworld boxworld/main.cxx
                        boxworld/images.cxx
                        boxworld/level.cxx
                        boxworld/levels.cxx
                        boxworld/boxworld.cxx)

target_link_libraries(boxworld drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(life life/main.cxx
                    life/life.cxx)

target_link_libraries(life drmfb32 ${DRM_LIBRARIES} Threads::Threads)

#--------------------------------------------------------------------------

add_executable(modes modes/modes.cxx)

target_link_libraries(modes drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------


add_executable(sphere sphere/main.cxx
                      sphere/sphere.cxx)

target_link_libraries(sphere drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(drmfb32test test/test.cxx)
target_link_libraries(drmfb32test drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(joysticktest test/testJoystick.cxx)
target_link_libraries(joysticktest drmfb32)
