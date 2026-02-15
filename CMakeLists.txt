cmake_minimum_required(VERSION 3.20)
project(mah_tracker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

find_package(OpenGL REQUIRED)

option(SDL2_VENDORED "Use the vendored library in ./SDL" ON)

if (NOT SDL2_VENDORED)
    find_package(SDL2 REQUIRED)
endif()

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(ROOT ${IMGUI_DIR})
set(RESIDFP_DIR ${CMAKE_SOURCE_DIR}/residfp)

add_library(imgui STATIC
	${ROOT}/imgui.cpp
	${ROOT}/imgui.h
	${ROOT}/imstb_rectpack.h
	${ROOT}/imstb_textedit.h
	${ROOT}/imstb_truetype.h
    ${ROOT}/imgui_demo.cpp
    ${ROOT}/imgui_draw.cpp
    ${ROOT}/imgui_internal.h
    ${ROOT}/imgui_tables.cpp
    ${ROOT}/imgui_widgets.cpp
)

add_library(residfp STATIC
    ${RESIDFP_DIR}/array.cpp
    ${RESIDFP_DIR}/Dac.cpp
    ${RESIDFP_DIR}/EnvelopeGenerator.cpp
    ${RESIDFP_DIR}/ExternalFilter.cpp
    ${RESIDFP_DIR}/Filter6581.cpp
    ${RESIDFP_DIR}/Filter8580.cpp
    ${RESIDFP_DIR}/Filter.cpp
    ${RESIDFP_DIR}/FilterModelConfig6581.cpp
    ${RESIDFP_DIR}/FilterModelConfig8580.cpp
    ${RESIDFP_DIR}/FilterModelConfig.cpp
    ${RESIDFP_DIR}/Integrator6581.cpp
    ${RESIDFP_DIR}/Integrator8580.cpp
    ${RESIDFP_DIR}/OpAmp.cpp
    ${RESIDFP_DIR}/SID.cpp
    ${RESIDFP_DIR}/Spline.cpp
    ${RESIDFP_DIR}/WaveformCalculator.cpp
    ${RESIDFP_DIR}/WaveformGenerator.cpp
    ${RESIDFP_DIR}/resample/SincResampler.cpp
)


target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${ROOT}>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(residfp PUBLIC
    $<BUILD_INTERFACE:${RESIDFP_DIR}>
)


set(INSTALL_TARGETS imgui)
set(INSTALL_HEADERS ${ROOT}/imgui.h ${ROOT}/imconfig.h ${RESIDFP_DIR}/siddefs-fp.h)

foreach(BACKEND opengl3 sdl2)
	set(NAME imgui_impl_${BACKEND})
	set(HEADER ${ROOT}/backends/${NAME}.h)
	add_library(${NAME} STATIC ${ROOT}/backends/${NAME}.cpp ${HEADER})
	target_link_libraries(${NAME} PUBLIC imgui)
	target_include_directories(${NAME} PUBLIC
	    $<BUILD_INTERFACE:${ROOT}/backends>
	    $<INSTALL_INTERFACE:include>)
	LIST(APPEND INSTALL_TARGETS ${NAME})
	LIST(APPEND INSTALL_HEADERS ${HEADER})
endforeach()

target_sources(imgui_impl_opengl3 PRIVATE ${ROOT}/backends/imgui_impl_opengl3_loader.h)
target_link_libraries(imgui_impl_opengl3 PRIVATE OpenGL::GL)

add_subdirectory(libsndfile EXCLUDE_FROM_ALL)

if(SDL2_VENDORED)
    add_subdirectory(SDL EXCLUDE_FROM_ALL)
    target_link_libraries(imgui_impl_sdl2 PRIVATE SDL2::SDL2)
else()
    target_link_libraries(imgui_impl_sdl2 PRIVATE SDL2::SDL2-static)
endif()


add_compile_definitions(ImTextureID=void*)
add_compile_options(-O2 -g)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(
    mah_tracker 
    main.cpp
	pattern.cpp
	orders.cpp
	instrument_edit.cpp
	player.cpp
    file_ops.cpp
)

if(SDL2_VENDORED)
    target_link_libraries(mah_tracker
        PRIVATE residfp imgui imgui_impl_sdl2 imgui_impl_opengl3 SDL2::SDL2 SDL2::SDL2main SndFile::sndfile
    )
else()
    target_link_libraries(mah_tracker
        PRIVATE residfp imgui imgui_impl_sdl2 imgui_impl_opengl3 SDL2::SDL2-static SDL2::SDL2main SndFile::sndfile
    )
endif()