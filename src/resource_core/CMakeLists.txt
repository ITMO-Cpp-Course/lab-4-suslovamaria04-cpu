add_library(resource_core
    src/resource/file_handle.cpp
    src/resource/resource_error.cpp
    src/resource/resource_manager.cpp
)

target_include_directories(resource_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_link_libraries(resource_core PRIVATE project_warnings)

if (TARGET project_sanitizers)
    target_link_libraries(resource_core PRIVATE project_sanitizers)
endif ()