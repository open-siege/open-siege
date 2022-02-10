from conans import ConanFile, CMake, tools
import os.path
import sys
import shutil

# TODO find a more permanent solution
imagemagick_url = "https://transfer.sh/btgQI6/ImageMagick.zip"

class LocalConanFile(ConanFile):
    settings = "os", "compiler", "build_type", "arch", "arch_build"
    build_requires = "cmake/3.22.0"

    def requirements(self):
        if os.path.exists("./cmake"):
            shutil.rmtree("./cmake")

        profile = "default"

        if "--profile" in sys.argv:
            profile = sys.argv[sys.argv.index("--profile") + 1]
            profile = os.path.abspath(profile) if profile != "default" else profile

        commands = [
            "cd 3space-studio",
            "pip3 install -r requirements.txt",
            "cd ..",
            "cd siege-shell",
            "pip3 install -r requirements.txt"
        ]

        self.run(" && ".join(commands), run_environment=True)

        targets = ["tools", "siege-shell", "3space-studio", "extender"]

        commands = [
            "cd 3space",
            f"conan install . --profile {profile} -s build_type={self.settings.build_type} -s arch={self.settings.arch} --build=missing",
            "conan export .",
            "cd ..",
            "mkdir cmake",
            "cd cmake",
            f"conan install 3space/0.5.1@/ -g cmake_find_package --profile {profile} -s build_type={self.settings.build_type} -s arch={self.settings.arch} --build=missing"
        ]
        self.run(" && ".join(commands), run_environment=True)


        for target in targets:
            commands = [
                f"cd {target}",
                f"conan install . --profile {profile} -s build_type={self.settings.build_type} -s arch={self.settings.arch} --build=missing"
            ]
            self.run(" && ".join(commands), run_environment=True)

        if not os.path.exists("ImageMagick.zip"):
            tools.download(imagemagick_url, "ImageMagick.zip")

        if not os.path.exists("ImageMagick"):
            tools.unzip("ImageMagick.zip", "ImageMagick")


    def _configure_cmake(self):
        magick_home = os.path.abspath(f"ImageMagick/ImageMagick-{self.settings.arch_build}")
        os.environ["PATH"] += os.pathsep + magick_home + os.sep
        os.environ["MAGICK_HOME"] = magick_home
        os.environ["MAGICK_CODER_MODULE_PATH"] = magick_home + os.sep + "modules" + os.sep + "coders"
        cmake = CMake(self)
        cmake.definitions["CONAN_CMAKE_SYSTEM_PROCESSOR"] = self.settings.arch
        cmake.definitions["CI"] = os.environ["CI"] if "CI" in os.environ else "False"
        cmake.definitions["PYTHON_EXECUTABLE"] = sys.executable
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        return cmake

    def build(self):
        self._configure_cmake().build()

    def package(self):
        git = tools.Git(folder="wiki")
        git.clone("https://github.com/StarsiegePlayers/3space-studio.wiki.git", shallow=True)
        tools.rmdir("wiki/.git")
        self.copy(pattern="LICENSE", src=self.source_folder, dst="licenses")
        self.copy(pattern="README.md", src=self.source_folder, dst="res")
        self.copy(pattern="wiki/*.md", src=self.source_folder, dst="res")
        self.copy(pattern="game-support.md", src=self.source_folder, dst="res")
        tools.rmdir("wiki")
        self._configure_cmake().install()
