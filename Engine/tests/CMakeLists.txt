add_executable(MyrIDMapTests
    MyrIDMap.test.cpp
)

target_include_directories(MyrIDMapTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_link_libraries(MyrIDMapTests PRIVATE
    Catch2::Catch2WithMain
)

add_test(NAME MyrIDMapTests COMMAND MyrIDMapTests)

add_executable(MyrIDArrayTests
    MyrIDArray.test.cpp
)

target_include_directories(MyrIDArrayTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_link_libraries(MyrIDArrayTests PRIVATE
    Catch2::Catch2WithMain
    MyriadEngine
)

add_test(NAME MyrIDArrayTests COMMAND MyrIDArrayTests)
