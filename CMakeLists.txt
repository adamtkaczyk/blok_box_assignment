cmake_minimum_required(VERSION 3.25.1)

set(CMAKE_CXX_STANDARD 17)

project(blok_box_assignment)

enable_testing()

find_package(Boost REQUIRED system)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)

add_executable(blok_box_assignment
    main.cpp
    marketdata.cpp
    session.cpp
    partial_book_update.cpp
    storage.cpp
    storage_item.cpp)

target_include_directories(blok_box_assignment 
    PRIVATE
        ${Boost_INCLUDE_DIRS} 
        ${OPENSSL_INCLUDE_DIR})

target_link_libraries(blok_box_assignment 
    ${Boost_LIBRARIES}
    ${OPENSSL_SSL_LIBRARY}
    ${OPENSSL_CRYPTO_LIBRARY}
    fmt::fmt-header-only
    nlohmann_json::nlohmann_json)
