message(STATUS "include UbpaInit.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/UbpaBasic.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/UbpaBuild.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/UbpaDownload.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/UbpaGit.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/UbpaPackage.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/UbpaQt.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/UbpaTool.cmake")

# ---------------------------------------------------------

macro(Ubpa_InitProject)
set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  Ubpa_Path_Back(root ${CMAKE_INSTALL_PREFIX} 1)
  set(CMAKE_INSTALL_PREFIX "${root}/Ubpa" CACHE PATH "install prefix" FORCE)
endif()

set("Ubpa_BuildTest_${PROJECT_NAME}" TRUE CACHE BOOL "build tests of ${PROJECT_NAME}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/lib")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# natvis
if(MSVC AND EXISTS "${PROJECT_SOURCE_DIR}/config/natvis.in")
  if(NOT TARGET Ubpa_natvis)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/NatvisEmpty.cpp" "// generated by UCMake for natvis\nint main(){ return 0; }\n")
    add_executable(Ubpa_natvis "${CMAKE_CURRENT_BINARY_DIR}/NatvisEmpty.cpp")
  endif()
  
  Ubpa_PackageName(package_name)
  set(natvis_file "${CMAKE_CURRENT_BINARY_DIR}/${package_name}.natvis")
  set(CONFIG_UCMAKE_NATVIS "<!-- \n    Generated by UCMake ${UCMake_VERSION} for natvis\n    Any changes to this file will be overwritten by the next CMake run\n-->")
  configure_file("${PROJECT_SOURCE_DIR}/config/natvis.in" ${natvis_file})
  target_sources(Ubpa_natvis PRIVATE ${natvis_file})
  install(
    FILES ${natvis_file}
    DESTINATION "${package_name}/cmake"
  )
endif()
endmacro()
