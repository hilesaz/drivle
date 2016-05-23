project(drivled)

include_directories(
    ${drivled_SOURCE_DIR}/../libdrivle/include
)

file (GLOB DRIVLED_SRC ${drivled_SOURCE_DIR}/src/*.c)

add_executable(drivled ${DRIVLED_SRC})
