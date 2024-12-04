cmake_minimum_required(VERSION 3.17)
project(tryemad2)

set(CMAKE_CXX_STANDARD 14)

add_executable(tryemad2 Fig3.cpp getBMP.cpp)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)



target_link_libraries(tryemad2  OpenGL::GL  GLUT::GLUT  GLEW::GLEW    -lfreeGLUT  GLU32   )