project(test-P10-linear-advection-dg-adapt)

add_executable(${PROJECT_NAME} main.cpp ../definitions.cpp)

if(NOT MSVC)
  set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_FLAGS ${HERMES_FLAGS})
endif()

target_link_libraries(${PROJECT_NAME} ${HERMES2D})

set(BIN ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})
add_test(test-linear-advection-dg-adapt ${BIN})
