function(add_examples_executable executable_name field)
    add_executable(${executable_name} ${field}/${executable_name}.cpp)
    target_link_libraries(${executable_name}
        ${PROJECT_NAME}
        pthread
    )
    message(">>>> Added Executable: ${executable_name} !")
    install(FILES ${PROJECT_BINARY_DIR}/examples/${executable_name} DESTINATION ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/bin/examples)
endfunction()

function(add_tests_executable executable_name field)
    add_executable(${executable_name} ${field}/${executable_name}.cpp)
    target_link_libraries(${executable_name}
        ${PROJECT_NAME}
        pthread
        libgtest
        libgmock
    )
    message(">>>> Added Executable: ${executable_name} !")
    install(FILES ${PROJECT_BINARY_DIR}/tests/${executable_name} DESTINATION ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/bin/tests)
endfunction()
