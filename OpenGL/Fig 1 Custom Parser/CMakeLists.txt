cmake_minimum_required(VERSION 3.17)
project(tryHollow)

set(CMAKE_CXX_STANDARD 14)


add_executable(tryHollow Fig1.cpp getBMP.cpp)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)



target_link_libraries(tryHollow  OpenGL::GL  GLUT::GLUT  GLEW::GLEW    -lfreeGLUT  GLU32   )