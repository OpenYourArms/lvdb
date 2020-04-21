
set(this lvdbTest)
add_subdirectory(googletest)

set(sources
        lvdbTests.cpp
        )

add_executable(${this} ${sources})

target_link_libraries(${this} PUBLIC
        gtest_main
        SrcLibs
        )

add_test(
        NAME ${this}
        COMMAND ${this}
)

