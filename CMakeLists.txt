cmake_minimum_required(VERSION 3.10)
project(drmfb32)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(BSD REQUIRED libbsd)
pkg_check_modules(DRM REQUIRED libdrm)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

#--------------------------------------------------------------------------

find_package(Freetype)
pkg_check_modules(TURBOJPEG libturbojpeg)

#--------------------------------------------------------------------------

add_library(drmfb32 STATIC libdrmfb32/drmMode.cxx
                           libdrmfb32/fileDescriptor.cxx
                           libdrmfb32/framebuffer8880.cxx
                           libdrmfb32/image8880.cxx
                           libdrmfb32/image8880Font8x16.cxx
                           libdrmfb32/image8880Graphics.cxx
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

target_link_libraries(info drmfb32 ${BSD_LIBRARIES}
                                   ${DRM_LIBRARIES}
                                   ${FREETYPE_LIBRARIES})
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

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thread-pool/include/BS_thread_pool.hpp)

add_executable(life life/main.cxx
                    life/life.cxx)

target_link_libraries(life drmfb32 ${DRM_LIBRARIES} Threads::Threads)

endif()

#--------------------------------------------------------------------------

add_executable(modes modes/modes.cxx)

target_link_libraries(modes drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(puzzle-15 puzzle-15/main.cxx
                         puzzle-15/images.cxx
                         puzzle-15/puzzle.cxx)

target_link_libraries(puzzle-15 drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

if (TURBOJPEG_FOUND)
add_executable(showjpeg showjpeg/showjpeg.cxx)
target_link_libraries(showjpeg drmfb32 ${DRM_LIBRARIES}
                                       ${TURBOJPEG_LIBRARIES})
endif()

#--------------------------------------------------------------------------

add_executable(showqoi showqoi/showqoi.cxx)
target_link_libraries(showqoi drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(sphere sphere/main.cxx
                      sphere/sphere.cxx)

target_link_libraries(sphere drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

if (TURBOJPEG_FOUND)
add_executable(slideshow slideshow/slideshow.cxx
                         slideshow/viewer.cxx)

target_link_libraries(slideshow drmfb32 ${DRM_LIBRARIES}
                                        ${TURBOJPEG_LIBRARIES})
endif()

#--------------------------------------------------------------------------

add_executable(test test/test.cxx)
target_link_libraries(test drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(testCircle test/testCircle.cxx)
target_link_libraries(testCircle drmfb32 ${DRM_LIBRARIES})

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
target_link_libraries(testResize drmfb32 ${DRM_LIBRARIES})

#--------------------------------------------------------------------------

add_executable(joysticktest test/testJoystick.cxx)
target_link_libraries(joysticktest drmfb32)

