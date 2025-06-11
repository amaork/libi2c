cmake_minimum_required(VERSION 3.22)

project(libi2c
	VERSION 0.2.1
	LANGUAGES C CXX
)

option(PYTHON_BINDINGS "Enable Python bindings compilation" OFF)
option(BUILD_EXAMPLES "Build library usage examples" OFF)

if(NOT LINUX)
	message(FATAL_ERROR "Non Linux build is not supported")
endif()

add_library(libi2c
	src/i2c.c
)
target_include_directories(libi2c
	PUBLIC
		include/
)
target_compile_definitions(libi2c
	PUBLIC
		"LIBI2C_VERSION=\"${CMAKE_PROJECT_VERSION}\""
)
target_compile_options(libi2c
	PRIVATE
		-Wall -Wextra
)
set_target_properties(libi2c
	PROPERTIES
		PREFIX ""
)

install(TARGETS libi2c)
install(DIRECTORY include/
	TYPE INCLUDE
)

if(PYTHON_BINDINGS)
	find_package(Python COMPONENTS Interpreter Development)

	add_library(pylibi2c SHARED
		src/pyi2c.c
	)
	target_link_libraries(pylibi2c
		PUBLIC
			Python::Python
			libi2c
	)
	set_target_properties(pylibi2c
		PROPERTIES
			PREFIX ""
	)
	install(TARGETS pylibi2c DESTINATION "${Python_SITEARCH}/")

	enable_testing()
	add_test(NAME python_bindings_test
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMAND ${Python_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tests/test_pylibi2c.py"
	)
	set_property(TEST python_bindings_test
		PROPERTY
			ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}"
	)
endif()

if(BUILD_EXAMPLES)
	add_subdirectory(example)
endif()

include(CPack)
