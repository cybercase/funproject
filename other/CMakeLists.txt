PROJECT(funproject)

add_executable(concurrent concurrent.cpp)
target_link_libraries(concurrent pthread)

add_executable(sorting_priorities sorting_priorities.cpp)

