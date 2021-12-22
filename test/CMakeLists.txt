find_package(DOCTEST)
if(FOUND_DOCTEST)
    # nothing
else()
  
    # Download and unpack doctest at configure time
    configure_file(downloadDoctest.cmake.in doctest-download/CMakeLists.txt)
   
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doctest-download )
    if(result)
        message(FATAL_ERROR "CMake step for doctest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doctest-download )
    if(result)
        message(FATAL_ERROR "Build step for doctest failed: ${result}")
    endif()

    set(DOCTEST_INCLUDE_DIR   ${CMAKE_CURRENT_BINARY_DIR}/doctest-src/doctest)

endif()

set(${PROJECT_NAME}_TEST_TARGET test_${PROJECT_NAME})



# all tests
set(${PROJECT_NAME}_TESTS
    test_btree_config.cpp
)

add_executable( ${${PROJECT_NAME}_TEST_TARGET}
    main.cpp
    ${${PROJECT_NAME}_TESTS}
)

target_include_directories(  ${${PROJECT_NAME}_TEST_TARGET} PRIVATE 
    "$<BUILD_INTERFACE:${DOCTEST_INCLUDE_DIR}>"
)

add_custom_target(cpp-test COMMAND ${${PROJECT_NAME}_TEST_TARGET}  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test" )
add_dependencies(cpp-test ${${PROJECT_NAME}_TEST_TARGET} )