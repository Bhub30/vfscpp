set(HEADER_DIR "../include/")
include_directories(${HEADER_DIR})

add_library(
  ${PROJECT_NAME} STATIC
  "FileSystem.cpp"
  "RegularFile.cpp"
)
target_include_directories(${PROJECT_NAME} PRIVATE ${HEADER_DIR})
