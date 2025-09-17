
enable_testing()


configure_file(${PROJECT_SOURCE_DIR}/example/resource/input_recipe.txt ${PROJECT_BINARY_DIR}/test/ COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/example/resource/output_recipe.txt ${PROJECT_BINARY_DIR}/test/ COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/dependencies/googletest/bin/Debug/gtest.dll ${PROJECT_BINARY_DIR}/test/ COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/dependencies/googletest/bin/Debug/gtest_main.dll ${PROJECT_BINARY_DIR}/test/ COPYONLY)


find_library(GTEST_LIB gtest.lib ${PROJECT_SOURCE_DIR}/dependencies/googletest/lib/Debug/)
find_library(GTEST_MAIN_LIB gtest_main.lib ${PROJECT_SOURCE_DIR}/dependencies/googletest/lib/Debug/)


file(GLOB SOURCES *.cpp)

foreach(SOURCE ${SOURCES})
    get_filename_component(ELITE_SDK_TEST_NAME ${SOURCE} NAME_WE)
    add_executable(${ELITE_SDK_TEST_NAME} ${SOURCE})
    target_include_directories(
        ${ELITE_SDK_TEST_NAME} 
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/
        ${PROJECT_SOURCE_DIR}/include/Common
        ${PROJECT_SOURCE_DIR}/include/Elite
        ${PROJECT_SOURCE_DIR}/include/Control
        ${PROJECT_SOURCE_DIR}/dependencies/googletest/include
    )
    target_link_libraries(
        ${ELITE_SDK_TEST_NAME}
        elite-cs-series-sdk::static
        # GTest::gtest
        ${SYSTEM_LIB}
        ${GTEST_LIB}
        ${GTEST_MAIN_LIB}
    )
    # # 根据配置链接不同版本的库（Debug/Release）
    # target_link_libraries(${ELITE_SDK_TEST_NAME} PRIVATE
    #     $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/dependencies/googletest/lib/Debug/gtest.lib>   # Debug模式
    #     $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/dependencies/googletest/lib/Release/gtest.lib>  # Release模式
    #     $${PROJECT_SOURCE_DIR}/dependencies/googletest/lib/Debug/gtest_main.lib  # 包含main()的入口库
    # )

    # Windows: 复制DLL到可执行文件目录
    # if(WIN32)
    #     add_custom_command(TARGET ${ELITE_SDK_TEST_NAME} POST_BUILD
    #         COMMAND ${CMAKE_COMMAND} -E copy
    #             "$${PROJECT_SOURCE_DIR}/dependencies/googletest/bin/Debug/gtest.dll"
    #             $<TARGET_FILE_DIR:${ELITE_SDK_TEST_NAME}>
    #         COMMAND ${CMAKE_COMMAND} -E copy
    #             "${PROJECT_SOURCE_DIR}/dependencies/googletest/bin/Debug/gtest_main.dll"
    #             $<TARGET_FILE_DIR:${ELITE_SDK_TEST_NAME}>
    #     )
    # endif()


    target_link_directories(
        ${ELITE_SDK_TEST_NAME} 
        PRIVATE ${CMAKE_BINARY_DIR}
    )
endforeach()
