cmake_minimum_required(VERSION 3.12)
project(string_template_tests CXX)

# ${CMAKE_PROJECT_NAME}: project name of the root CMakeLists.txt
# ${PROJECT_NAME}: project name of the current CMakeLists.txt
add_executable(${PROJECT_NAME}
  string_template.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  StrTpl::StrTpl
  StrTplTestsConfig
  Catch2::Catch2WithMain
)

add_test(${PROJECT_NAME} ${PROJECT_NAME})
