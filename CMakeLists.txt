cmake_minimum_required(VERSION 3.27.0)

project(SQITS VERSION 1.0)

add_subdirectory(library)

add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} PUBLIC UTILITY)

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/library")