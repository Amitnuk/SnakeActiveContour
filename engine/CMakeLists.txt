find_package(OpenCV REQUIRED)


add_library(${ENGINE_TARGET_NAME} STATIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine.cpp
)           


target_include_directories(${ENGINE_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC ${OpenCV_LIBS})


if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Release mode")
  target_compile_options(${ENGINE_TARGET_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-lineinfo>)

else() 
  message(STATUS "Debug mode enabled")
  target_compile_options(${ENGINE_TARGET_NAME} PUBLIC 
                                            $<$<COMPILE_LANGUAGE:CXX>:-g>  
                                            $<$<COMPILE_LANGUAGE:CXX>:-Wall>
                                            $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
					                        $<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
endif()
