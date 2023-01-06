# CMakeList.txt : CMake project for GridOfLife, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("gridoflife")


set(SOURCE_DIR "src")
set(DEPENDENCIES_DIR "dependencies")


add_executable (gridoflife)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gridoflife PROPERTY CXX_STANDARD 20)
  set_property(TARGET gridoflife PROPERTY CMAKE_CXX_EXTENSIONS OFF)
  set_property(TARGET gridoflife PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
endif()


target_compile_options(
	gridoflife
	PRIVATE
	-fno-exceptions
	-Wall)

target_sources(
	gridoflife
	PRIVATE
    ${SOURCE_DIR}/gridoflife.cpp
    ${SOURCE_DIR}/internal_sdl_state.cpp
)
	
target_include_directories(
	gridoflife
	PRIVATE
	${SOURCE_DIR})

########################################################################
#                               IMGUI                                  #
########################################################################
########################################################################
set(imgui_SOURCE_DIR "${DEPENDENCIES_DIR}/imgui")

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/imgui_internal.h
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}/
    ${imgui_SOURCE_DIR}/backends)

target_link_libraries(gridoflife PRIVATE imgui)

########################################################################
#                               SDL2                                   #
########################################################################
########################################################################

# documentation: https://github.com/libsdl-org/SDL/blob/release-2.26.x/docs/README-cmake.md

option(USE_SDL_VENDORED "Use SDL2 vendored libraries" OFF)

if(USE_SDL_VENDORED)
    add_subdirectory(${DEPENDENCIES_DIR}/SDL2 EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
    
    # 1. Look for a SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available 
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
endif()

if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(gridoflife PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(gridoflife PRIVATE SDL2::SDL2-static)