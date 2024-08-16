from conan import ConanFile
from conan.tools.cmake import CMake

class NcursesDinoRunnerConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("ncurses/6.3")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
