cmake_minimum_required(VERSION 3.10)
project(network_latency_checker)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)

set(CLIENT_SOURCE_FILES 
  src/client.cpp
)

set(SERVER_SOURCE_FILES 
  src/server.cpp
)

# 타겟 실행 파일 정의
add_executable(${PROJECT_NAME}_client 
  ${CLIENT_SOURCE_FILES}
)
add_executable(${PROJECT_NAME}_server 
  ${SERVER_SOURCE_FILES}
)

target_link_libraries(${PROJECT_NAME}_client  
  Threads::Threads
)
target_link_libraries(${PROJECT_NAME}_server  
  Threads::Threads
)