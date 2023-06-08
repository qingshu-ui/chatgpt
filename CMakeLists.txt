cmake_minimum_required(VERSION 3.0.0)
project(chatgpt VERSION 1.0.0)

aux_source_directory(src CHATGPT_SOURCE_FILES)
add_library(${PROJECT_NAME} SHARED ${CHATGPT_SOURCE_FILES})

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# JsonCpp library include
find_package(JsonCpp CONFIG REQUIRED)
if(JsonCpp_FOUND)
    message("-- JsonCpp found")
    target_link_libraries(${PROJECT_NAME} PRIVATE JsonCpp::JsonCpp)
    target_include_directories(${PROJECT_NAME} PRIVATE JsonCpp_INCLUDE_DIR)
endif()

# Protobuf library include
find_package(Protobuf CONFIG REQUIRED)
if(Protobuf_FOUND)   
    message("-- Protobuf found")
    target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::protobuf)
    target_include_directories(${PROJECT_NAME} PRIVATE Protobuf_INCLUDE_DIR)
endif()

# Drogon library include
find_package(Drogon CONFIG REQUIRED)
if(Drogon_FOUND)
    message("-- Drogon found")
    target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
    target_include_directories(${PROJECT_NAME} PRIVATE Drogon_INCLUDE_DIR)
endif()

# Libcurl library include
find_package(CURL CONFIG REQUIRED)
if(CURL_FOUND)
    message("-- curl found")
    target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
    target_include_directories(${PROJECT_NAME} PRIVATE CURL_INCLUDE_DIR)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE PluginDev)
target_link_libraries(${PROJECT_NAME} PRIVATE Post_Protocol) 
