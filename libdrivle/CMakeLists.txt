project(libdrivle)

file (GLOB LIBDRIVLE_SRC ${libdrivle_SOURCE_DIR}/src/*.c)

include_directories(
    ${libdrivle_SOURCE_DIR}/include
    ${libdrivle_SOURCE_DIR}/src
)

add_library(drivle STATIC ${LIBDRIVLE_SRC})


file (GLOB UNITTEST_SRC ${libdrivle_SOURCE_DIR}/test/*.c)

add_executable(unittest ${UNITTEST_SRC})
target_link_libraries(unittest drivle)
