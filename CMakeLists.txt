#cmake_minimum_required(VERSION 3.0.0)


project(pureparameter VERSION 1.0.0)

#set (CMAKE_CXX_STANDARD 11)

add_library(lib${PROJECT_NAME} INTERFACE)

target_include_directories(lib${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include/)
target_sources(lib${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/pureparameter.h)

# see all file in generated ides (like xcode)
FILE(GLOB CORE_HEADERS include/*.h)
target_sources(lib${PROJECT_NAME} INTERFACE ${CORE_HEADERS})

target_link_libraries(lib${PROJECT_NAME} INTERFACE)

