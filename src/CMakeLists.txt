# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (servidor-simple-impresiones "main.cpp" "main.h" "check_printer.h" "check_printer.cpp" "pch.h" "logger.h")

target_precompile_headers(servidor-simple-impresiones PUBLIC "pch.h")

# Link to the librares
target_link_libraries(servidor-simple-impresiones PRIVATE cpprestsdk::cpprest)
target_link_libraries(servidor-simple-impresiones PRIVATE libusb::libusb)
target_link_libraries(servidor-simple-impresiones PRIVATE spdlog::spdlog)
target_link_libraries(servidor-simple-impresiones PRIVATE opencv::opencv)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET servidor-simple-impresiones PROPERTY CXX_STANDARD 20)
endif()

install(TARGETS servidor-simple-impresiones DESTINATION bin)