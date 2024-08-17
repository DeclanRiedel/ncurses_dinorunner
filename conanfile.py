from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class NcursesDinoRunnerConan(ConanFile):
    name = "ncurses_dinorunner"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("ncurses/6.5")

    def layout(self):
        cmake_layout(self, src_folder=".", build_folder="build")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()