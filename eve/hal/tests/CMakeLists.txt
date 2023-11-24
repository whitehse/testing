if(ENABLE_TESTING)
    set(TEST_SOURCES "main.c")
    set(TEST_INCLUDE_SOURCES "${PROJECT_SOURCE_DIR}/src/")

    add_executable(
      ${UNIT_TEST_NAME}
      ${TEST_SOURCES}
      ${TEST_INCLUDES}
    )

    target_include_directories(${UNIT_TEST_NAME} PUBLIC ${TEST_INCLUDE_SOURCES})
    #target_link_libraries(${UNIT_TEST_NAME} PUBLIC ${LIBRARY_NAME})
    target_link_libraries(${UNIT_TEST_NAME} PRIVATE unity)
    target_link_libraries(${UNIT_TEST_NAME} PRIVATE m)
    if(${ENABLE_WARNINGS})
        target_set_warnings(
            TARGET
            ${UNIT_TEST_NAME}
            ENABLE
            ON
            AS_ERRORS
            OFF)
    endif()

    if(ENABLE_COVERAGE)
        set(COVERAGE_MAIN "coverage")
        set(COVERAGE_EXCLUDES
            "${PROJECT_SOURCE_DIR}/app/*"
            "${PROJECT_SOURCE_DIR}/cmake/*"
            "${PROJECT_SOURCE_DIR}/docs/*"
            "${PROJECT_SOURCE_DIR}/external/*"
            "${PROJECT_SOURCE_DIR}/tests/*"
            "/usr/include/*")
        setup_target_for_coverage_lcov(
            NAME
            ${COVERAGE_MAIN}
            EXECUTABLE
            ${UNIT_TEST_NAME}
            DEPENDENCIES
            ${UNIT_TEST_NAME})
    endif()
endif()
