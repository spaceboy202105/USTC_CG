Ubpa_GetTargetName(tool "${PROJECT_SOURCE_DIR}/src/tool")
set(src
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
#	${PROJECT_SOURCE_DIR}/data/shaders/dn.vert
#	${PROJECT_SOURCE_DIR}/data/shaders/light_dn.frag)
	${PROJECT_SOURCE_DIR}/data/shaders/dn_displacement.vert
	${PROJECT_SOURCE_DIR}/data/shaders/light_dn_displacement.frag)
Ubpa_AddTarget(MODE "EXE" SOURCES ${src} LIBS "Ubpa::UGL_core;glfw;${tool}")
