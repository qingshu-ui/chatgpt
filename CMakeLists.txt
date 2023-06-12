cmake_minimum_required(VERSION 3.0.0)
project(chatgpt VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    add_compile_options(-MD)
endif()

aux_source_directory(src CHATGPT_SOURCE_FILES)
add_library(${PROJECT_NAME} SHARED ${CHATGPT_SOURCE_FILES})

find_package(Drogon CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)


add_subdirectory(protocol)
include_directories(protocol/src)
set(PROTOCOL_ADDED TRUE)

add_subdirectory(PluginDev)
include_directories(PluginDev/src)
set(PLUGINDEV_ADDED TRUE)

target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::protobuf)
target_link_libraries(${PROJECT_NAME} PRIVATE PluginDev)
target_link_libraries(${PROJECT_NAME} PRIVATE protocol) 