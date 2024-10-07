cmake_minimum_required(VERSION 3.10)

project(DbusExample)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)

include_directories(
    ${DBUS_INCLUDE_DIRS}
    src
)

add_executable(dbus_server src/dbus_server.c)
add_executable(dbus_client src/dbus_client.c)

target_link_libraries(dbus_server ${DBUS_LIBRARIES})
target_link_libraries(dbus_client ${DBUS_LIBRARIES})