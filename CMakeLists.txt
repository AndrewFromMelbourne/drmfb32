cmake_minimum_required(VERSION 3.10)
project(drmfb32)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(BSD REQUIRED libbsd)
pkg_check_modules(DRM REQUIRED libdrm)

if (CMAKE_BUILD_TYPE STREQUAL "")
set(CMAKE_BUILD_TYPE Release)
endif()

message("Build type ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-psabi")
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

#--------------------------------------------------------------------------

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak,undefined")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,leak,undefined")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,leak,undefined")
endif()

#--------------------------------------------------------------------------

find_package(Freetype)
pkg_check_modules(TURBOJPEG libturbojpeg)

#--------------------------------------------------------------------------

add_library(drmfb32 STATIC libdrmfb32/drmMode.cxx
                           libdrmfb32/fileDescriptor.cxx
                           libdrmfb32/framebuffer8880.cxx
                           libdrmfb32/image8880.cxx
                           libdrmfb32/image8880Font8x16.cxx
                           libdrmfb32/image8880Frames.cxx
                           libdrmfb32/image8880Graphics.cxx
                           libdrmfb32/image8880Process.cxx
                           libdrmfb32/image8880Qoi.cxx
                           libdrmfb32/interface8880.cxx
                           libdrmfb32/interface8880Font.cxx
                           libdrmfb32/joystick.cxx
                           libdrmfb32/rgb8880.cxx)

if (FREETYPE_FOUND)
target_include_directories(drmfb32 PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_sources(drmfb32 PRIVATE libdrmfb32/image8880FreeType.cxx)
endif()

if (TURBOJPEG_FOUND)
target_include_directories(drmfb32 PUBLIC ${TURBJPEG_INCLUDE_DIRS})
target_sources(drmfb32 PRIVATE libdrmfb32/image8880Jpeg.cxx)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thread-pool/include/BS_thread_pool.hpp)
set(WITH_BS_THREAD_POOL TRUE)
set(BS_THREAD_LIBRARIES Threads::Threads)
add_compile_definitions(WITH_BS_THREAD_POOL)
else()
message(WARNING "Not using BS Thread Pool")
endif()

include_directories(${PROJECT_SOURCE_DIR}/libdrmfb32
                    ${PROJECT_SOURCE_DIR}/thread-pool/include)
target_include_directories(drmfb32 PUBLIC ${DRM_INCLUDE_DIRS})
target_compile_options(drmfb32 PUBLIC ${DRM_CFLAGS_OTHER})
target_link_libraries(drmfb32 ${DRM_LIBRARIES})

set(EXTRA_LIBS ${EXTRA_LIBS} drmfb32)

#--------------------------------------------------------------------------

if (BSD_FOUND AND FREETYPE_FOUND)
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

target_link_libraries(info drmfb32 ${BSD_LIBRARIES}
                                   ${DRM_LIBRARIES}
                                   ${FREETYPE_LIBRARIES})
target_include_directories(info PUBLIC ${BSD_INCLUDE_DIRS})
target_compile_options(info PUBLIC ${BSD_CFLAGS_OTHER})

set_property(TARGET info PROPERTY SKIP_BUILD_RPATH TRUE)
install (TARGETS info RUNTIME DESTINATION bin)
endif()

#--------------------------------------------------------------------------

add_executable(boxworld boxworld/main.cxx
                        boxworld/level.cxx
                        boxworld/levels.cxx
                        boxworld/boxworld.cxx)

target_link_libraries(boxworld drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(joystickConfigure joystick/joystickConfigure.cxx)

target_link_libraries(joystickConfigure drmfb32)

#--------------------------------------------------------------------------

add_executable(life life/main.cxx
                    life/life.cxx)

target_link_libraries(life drmfb32 ${DRM_LIBRARIES}
                                   ${BS_THREAD_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(modes modes/modes.cxx)

target_link_libraries(modes drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(puzzle-15 puzzle-15/main.cxx
                         puzzle-15/puzzle.cxx)

target_link_libraries(puzzle-15 drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

if (TURBOJPEG_FOUND)
add_executable(showcam showcam/showcam.cxx
                       showcam/webcam.cxx)
target_link_libraries(showcam drmfb32 ${DRM_LIBRARIES}
                                      ${TURBOJPEG_LIBRARIES}
                                      ${BS_THREAD_LIBRARIES})
endif()

#--------------------------------------------------------------------------

if (TURBOJPEG_FOUND)
add_executable(showjpeg showjpeg/showjpeg.cxx)
target_link_libraries(showjpeg drmfb32 ${DRM_LIBRARIES}
                                       ${TURBOJPEG_LIBRARIES}
                                       ${BS_THREAD_LIBRARIES})
endif()

#--------------------------------------------------------------------------

add_executable(showqoi showqoi/showqoi.cxx)
target_link_libraries(showqoi drmfb32 ${DRM_LIBRARIES}
                                      ${BS_THREAD_LIBRARIES})


#--------------------------------------------------------------------------

add_executable(sphere sphere/main.cxx
                      sphere/sphere.cxx)

target_link_libraries(sphere drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

if (TURBOJPEG_FOUND)
add_executable(slideshow slideshow/slideshow.cxx
                         slideshow/viewer.cxx)

target_link_libraries(slideshow drmfb32 ${DRM_LIBRARIES}
                                        ${TURBOJPEG_LIBRARIES}
                                        ${BS_THREAD_LIBRARIES})
endif()

#--------------------------------------------------------------------------

add_executable(test test/test.cxx)
target_link_libraries(test drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(testCircle test/testCircle.cxx)
target_link_libraries(testCircle drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(testColourBlend test/testColourBlend.cxx)
target_link_libraries(testColourBlend drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

if (FREETYPE_FOUND)
add_executable(testft test/testft.cxx)
target_link_libraries(testft drmfb32
                             ${DRM_LIBRARIE}
                             ${FREETYPE_LIBRARIES})

add_executable(testft2 test/testft2.cxx)
target_link_libraries(testft2 drmfb32
                             ${DRM_LIBRARIE}
                             ${FREETYPE_LIBRARIES})
endif()

#--------------------------------------------------------------------------

add_executable(testPutImage test/testPutImage.cxx)
target_link_libraries(testPutImage drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(testResize test/testResize.cxx)
target_link_libraries(testResize drmfb32 ${DRM_LIBRARIES}
                                         ${BS_THREAD_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(joysticktest test/testJoystick.cxx)
target_link_libraries(joysticktest drmfb32)

