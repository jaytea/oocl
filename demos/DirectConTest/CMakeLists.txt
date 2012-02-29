project (DirectConTest)

include_directories (../../include) 
link_directories (../../lib) 

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${oocl_SOURCE_DIR}/bin)

add_executable (DirectConTest DirectConTest.cpp)

target_link_libraries (DirectConTest oocl) 