cmake_minimum_required(VERSION 3.22)

project(libi2c_examples)

add_executable(i2c_tools
	i2c_tools.c
)
target_link_libraries(i2c_tools
	PUBLIC
		libi2c
)

add_executable(i2c_without_internal_address
	i2c_without_internal_address.c
)
target_link_libraries(i2c_without_internal_address
	PUBLIC
		libi2c
)
install(TARGETS i2c_tools i2c_without_internal_address)
