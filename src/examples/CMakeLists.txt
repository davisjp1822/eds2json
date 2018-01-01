project(eds2json VERSION 1.0 LANGUAGES C)

add_executable(simple_example
    src/simple_example.c)

target_link_libraries(simple_example
    eds2json)