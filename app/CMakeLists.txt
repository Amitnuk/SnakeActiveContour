add_executable(${APP_TARGET_NAME} 
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_include_directories(${APP_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${APP_TARGET_NAME} PUBLIC ${ENGINE_TARGET_NAME})

