# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (servidor-simple-impresiones "main.cpp" "main.h" "check_printer.h" "check_printer.cpp" "pch.h" "logger.h" "utils.h" "utils.cpp")

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

# To install, for example, MSVC runtime libraries:
if (MSVC)
	include(InstallRequiredSystemLibraries)
	set(CPACK_SOURCE_GENERATOR "TGZ")
elseif (UNIX)
	set(CPACK_SOURCE_GENERATOR "DEB")
endif (MSVC)

# To install other/non-system 3rd party required libraries:
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
  @ONLY
)

install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)