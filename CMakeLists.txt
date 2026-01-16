cmake_minimum_required(VERSION 3.20)
project(mah_tracker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(ROOT ${IMGUI_DIR})

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

target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${ROOT}>
    $<INSTALL_INTERFACE:include>
)

set(INSTALL_TARGETS imgui)
set(INSTALL_HEADERS ${ROOT}/imgui.h ${ROOT}/imconfig.h ${YMFM_DIR}/ymfm.h ${YMFM_DIR}/ymfm_opn.h)

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
target_link_libraries(imgui_impl_sdl2 PRIVATE SDL2::SDL2-static)

add_compile_definitions(ImTextureID=void*)
add_compile_options(-O2)

add_executable(
    mah_tracker 
    main.cpp
)

target_link_libraries(mah_tracker
	PRIVATE imgui imgui_impl_sdl2 imgui_impl_opengl3 SDL2::SDL2-static SDL2::SDL2main
)
