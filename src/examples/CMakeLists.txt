project(eds2json VERSION 1.0 LANGUAGES C)

add_executable(simple_example
	src/simple_example.c)

target_link_libraries(simple_example
	eds2json)

find_package(Threads REQUIRED)

add_executable(threaded_example
	src/threaded_example.c)

target_link_libraries(threaded_example
	eds2json
	${CMAKE_THREAD_LIBS_INIT})
	