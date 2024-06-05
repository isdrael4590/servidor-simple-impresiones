# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (cliente-impresiones "main.cpp" "main.h")
target_link_libraries(cliente-impresiones PRIVATE cpprestsdk::cpprest)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET cliente-impresiones PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
