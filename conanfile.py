from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout

class ImpresionesReceta(ConanFile):
    name = "Servidor Simple de impresiones"
    version = "0.0.0.0"
    package_type = "application"

    # Optional metadata
    license = "Derechos Reservados, @josejacomeb"
    author = "Jose Jacome josejacomeb@gmail.com"
    url = "https://github.com/isdrael4590/servidor-simple-impresiones"
    description = "Simple servidor de impresiones de etiquetas para esterilizacion"
    topics = ("impresiones", "C++", "servidor")
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("cpprestsdk/2.10.19")

    #def layout(self):
    #    cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.user_presets_path = "ConanPresets.json"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()