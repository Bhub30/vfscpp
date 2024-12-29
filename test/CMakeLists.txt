find_package(GTest REQUIRED)
project(test)

set(HEADER_DIR "../include/")
include_directories(${HEADER_DIR})

enable_testing()

add_executable(
    FileSystemTest FileSystemTest.cpp
)
add_executable(
    RegularFileTest RegularFileTest.cpp
)

link_directories(${CMAKE_BINARY_DIR})
target_link_libraries(
    FileSystemTest vfs GTest::GTest GTest::Main
)
target_link_libraries(
    RegularFileTest vfs GTest::GTest GTest::Main
)

include(GoogleTest)
gtest_discover_tests(FileSystemTest)
gtest_discover_tests(RegularFileTest)
