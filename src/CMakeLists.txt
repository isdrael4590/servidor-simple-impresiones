# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (servidor-simple-impresiones "main.cpp" "main.h")
target_link_libraries(servidor-simple-impresiones PRIVATE cpprestsdk::cpprest)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET servidor-simple-impresiones PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
