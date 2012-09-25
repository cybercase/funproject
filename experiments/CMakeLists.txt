project(funproject)

if (${CUDA_FOUND})
    include(FindCUDA)
    cuda_add_executable( lev_distance lev_distance.cu )
else (${CUDA_FOUND})
    message( "No CUDA Toolkit found! some targets will not be built" )
endif (${CUDA_FOUND})

add_executable(concurrent concurrent.cpp)
target_link_libraries(concurrent pthread)

add_executable(sorting_priorities sorting_priorities.cpp)

