project(GL_Basics)
cmake_minimum_required(VERSION 2.8)

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

aux_source_directory(. SRC_LIST)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )

add_executable( ${PROJECT_NAME} ${SRC_LIST} )
if(NOT WIN32)
    target_link_libraries( ${PROJECT_NAME} -ldl )
endif()

find_package ( SDL2 REQUIRED )
include_directories( ${SDL2_INCLUDE_DIR}  )

if(NOT WIN32)
    target_link_libraries( ${PROJECT_NAME} "${SDL2_LIBRARY} -lm" )
else()
    target_link_libraries( ${PROJECT_NAME} "${SDL2_LIBRARY}" )
endif()

add_custom_command (
    TARGET ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/texture.tga ${CMAKE_CURRENT_BINARY_DIR}/texture.tga
    )
