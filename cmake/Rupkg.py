#!/bin/python3

import platform
import sys
import argparse
import os
import subprocess
import shutil
import multiprocessing
import importlib.util
from typing import List

supportPlatform = ["windows", "linux", "macos",
                "android", "ios", "emscripten", "tizen", "ns", "uwp",
                "avr", "pico"]

supportArchitecture = [ "default", "x64", "x86", "arm", "arm64", "wasm32", "wasm64" ]

supportGenerator = [ "default", "vs2022", "ninja", "xcode" ]

supportCompiler = [ "default", "clang", "gnu", "msvc", "appleclang" ]

supportConfiguration = ['Release', 'Debug']

supportPlatformArchitecture = {
    "windows": ["x64", "x86"],
    "linux": ["x64", "x86"],
    "macos": ["arm64", "x64"],
    "android": ["arm64", "x64", "arm", "x86"],
    "ios": ["arm64"],
    "emscripten": ["wasm32", "wasm64"],
    "tizen": ["arm", "x86"],
    "ns": ["arm64"],
    "uwp": ["arm64", "x64", "arm", "x86"],
    "avr": ["default"],
    "pico": ["default"]
}

supportPlatformGenerator = {
    "windows": ["vs2022", "ninja"],
    "linux": ["ninja"],
    "macos": ["ninja", "xcode"],
    "android": ["ninja"],
    "ios": ["ninja", "xcode"],
    "emscripten": ["ninja"],
    "tizen": ["ninja"],
    "ns": ["ninja"],
    "uwp": ["vs2022", "ninja"],
    "avr": ["ninja"],
    "pico": ["ninja"]
}

supportPlatformOnHost = {
    "windows":["windows", "android", "emscripten", "tizen", "ns", "uwp", "avr", "pico" ],
    "linux":["linux", "android", "emscripten", "avr"],
    "macos":["macos", "android", "ios", "emscripten", "avr"]
}

supportPlatformCompiler = {
    "windows": ["msvc", "clang"],
    "linux": ["gnu", "clang"],
    "macos": ["appleclang"],
    "android": ["clang"],
    "ios": ["appleclang"],
    "emscripten": ["clang"],
    "tizen": ["clang", "gnu"],
    "ns": ["clang"],
    "uwp": ["msvc"],
    "avr": ["gnu"],
    "pico": ["gnu"]
}

supportPlatformMCU = {
    "avr": ["atmega32u4", "atmega16u2"],
    "pico": ["RP2040"]
}

def _print_error(errmsg):
    print("Error: " + errmsg)

def _norm_cmake_path(path: str):
    return os.path.abspath(os.path.normpath(path)).replace("\\", "/")

def _get_host_platform():
    if 'Windows' in platform.system():
        return 'windows'
    elif 'Darwin' in platform.system():
        return 'macos'
    elif 'Linux' in platform.system():
        return 'linux'
    else:
        _print_error("Unknow platform: " + platform.system())
        sys.exit(-1)

def _get_host_architecture():
    if platform.machine() == 'x86_64' or platform.machine() == 'AMD64':
        return 'x64'
    elif platform.machine() == "aarch64" or platform.machine() == "arm64" or platform.machine() == "armv8l":
        return 'arm64'
    elif platform.machine() == "aarch64_be" or platform.machine() == "arm64_be" or platform.machine() == "armv8b":
        return 'arm64b'
    elif platform.machine() == "armv7l":
        return 'arm'
    elif platform.machine() == "armv7b":
        return 'armb'
    elif platform.machine() == "i686" or platform.machine() == "i386":
        return 'x86'
    else:
        _print_error("Unknow architecture: " + platform.machine())
        sys.exit(-1)

def _get_default_arch(platform: str):
    if platform == _get_host_platform():
        return _get_host_architecture()
    return supportPlatformArchitecture[platform][0]

def _get_default_generator(platform: str):
    return supportPlatformGenerator[platform][0]

def _find_vcvarsall(generator: str):
    if _get_host_platform() != "windows":
        _print_error("vcvarsall is only for windows")
        sys.exit(-1)

    version = None
    if generator == "vs2022":
        version = "[17.0, 18.0)"

    vswhere = os.path.join(os.environ['ProgramFiles(x86)'], 'Microsoft Visual Studio', 'Installer', 'vswhere.exe')
    if not os.path.exists(vswhere):
        _print_error("vswhere not found")
        sys.exit(-1)
    vswhere_command = [vswhere,
        "-latest",
        "-products", "*",
        "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
        "-property", "installationPath"
    ]
    if version is not None:
        vswhere_command.extend(["-version", version])
    proc = subprocess.Popen(vswhere_command, stdout=subprocess.PIPE)
    output = proc.communicate()[0]
    if proc.returncode != 0:
        _print_error("vswhere failed")
        sys.exit(-1)
    if len(output) == 0:
        _print_error("vswhere failed")
        sys.exit(-1)
    vcInstallPath = output.decode("utf-8").split("\r\n")[0]
    vcvarsall = os.path.join(vcInstallPath, "VC", "Auxiliary", "Build", "vcvarsall.bat")
    if not os.path.exists(vcvarsall):
        _print_error("vcvarsall not found")
        sys.exit(-1)
    return vcvarsall

def _get_vcvarsall_arch(architecture: str):
    target_type = architecture
    if target_type == "x64":
        target_type = "amd64"
    
    host_type = _get_host_architecture()
    if host_type == "x64":
        host_type = "amd64"

    if host_type == target_type:
        return host_type
    else:
        return host_type + "_" + target_type

class GenerateOptions:
    android_ndk: str
    emscripten: str
    switch_sdk: str
    tizen_sdk: str
    ios_dev_team: str

class CMakeCommand:
    def __init__(self, generator, sourceDir, binaryDir):
        self._Generator = generator
        self._SourceDir = sourceDir
        self._BinaryDir = binaryDir
        self._Parameters = {}
        self._GenerateCommandExtra = []
        self._PrefixCommand = []

    def addPrefixCommand(self, prefixCommand):
        self._PrefixCommand.extend(prefixCommand)

    def addDict(self, d):
        self._Parameters.update(d)
        return self

    def add(self, k, v):
        self._Parameters[k] = v
        return self

    def useExternalMake(self, m):
        self.add("CMAKE_MAKE_PROGRAM", m)
        return self

    def externalToken(self, a):
        self._GenerateCommandExtra.append(a)
        return self

    def getBinaryDir(self):
        return self._BinaryDir

    def generateCommand(self, cmake="cmake"):
        tokens = [
            cmake,
            "-S",
            self._SourceDir,
            "-B",
            self._BinaryDir,
            '-G',
            self._Generator
        ]

        tokens.extend(self._GenerateCommandExtra)

        tokens = self._PrefixCommand + tokens

        for (k, v) in self._Parameters.items():
            tokens.append("-D")
            tokens.append("%s=%s" % (k, str(v)))
        return tokens

class CMakeCommandGenerator:
    def __init__(self, platform, architecture, lto):
        self._platform = platform
        self._lto = lto

        if architecture == "default":
            architecture = _get_default_arch(platform)
        self._architecture = architecture

    def getPlatform(self):
        return self._platform
    def getLto(self):
        return self._lto
    def getArchitecture(self):
        return self._architecture
    def getTriple(self):
        triple = self.getPlatform() + "-" + self.getArchitecture()
        if self.getLto():
            triple = triple + "-lto"
        return triple

    def errorMessage(self, message):
        _print_error(message)

    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        pass
    def beforeGenerate(self):
        pass
    def afterGenerate(self):
        pass
    def checkGenerate(self, ext: GenerateOptions):
        return True

    def configBuildCommand(self, build_path: str, cmd: List[str]):
        pass
    
    def createGenerateCommand(self, srcRoot: str, buildRoot: str, generator: str, ext: GenerateOptions):
        if self.getArchitecture() not in supportPlatformArchitecture[self.getPlatform()]:
            self.errorMessage("Unsupport architecture: " + self.getArchitecture())
            sys.exit(-1)
        if self.checkGenerate(ext) == False:
            sys.exit(-1)
        
        if generator == "default":
            generator = _get_default_generator(self.getPlatform())
        if generator not in supportPlatformGenerator[self.getPlatform()]:
            self.errorMessage("Unsupport generator: " + generator)
            sys.exit(-1)
        _generatorMapping = {
            "vs2022":'Visual Studio 17 2022',
            "ninja" : 'Ninja Multi-Config',
            "xcode": 'Xcode'
        }
        cmd = CMakeCommand(_generatorMapping[generator], srcRoot, os.path.join(buildRoot, self.getTriple()))

        if self.getLto():
            cmd = cmd.add("RUPKG_ENABLE_LTO", "on")
            
        self.configGenerateCommand(cmd, generator, ext)

        return cmd
    
    def createBuildCommand(self, buildRoot: str, config: List[str], parallel: str, target: List[str]):
        result = []
        build_path = os.path.join(buildRoot, self.getTriple())
        if 'Release' in config and 'Debug' in config and os.path.exists(os.path.join(build_path, 'build.ninja')):
            build_command = ["ninja", "-C", build_path, "-j", str(parallel)]
            build_command.extend(target)
            self.configBuildCommand(build_path, build_command)
            result.append(build_command)
        else:
            for config in config:
                build_command = ["cmake", "--build", build_path, "--config", config, "-j", str(parallel)]
                for t in target:
                    build_command.append("--target")
                    build_command.append(t)
                self.configBuildCommand(build_path, build_command)
                result.append(build_command)
        return result

class CMakeMicroControllerCommandGenerator(CMakeCommandGenerator):
    def __init__(self, platform, mcu):
        super().__init__(platform, "default", False)
        self._mcu = mcu

    def getTriple(self):
        return self.getPlatform() + "-" + self.getMCU()

    def getMCU(self):
        return self._mcu

    def createGenerateCommand(self, srcRoot: str, buildRoot: str, generator: str, ext: GenerateOptions):
        if self.getMCU() not in supportPlatformMCU[self.getPlatform()]:
            self.errorMessage("Unsupport mcu: " + self.getMCU())
            sys.exit(-1)
        return super().createGenerateCommand(srcRoot, buildRoot, generator, ext)

class AndroidCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("android", architecture, lto)
    def checkGenerate(self, ext):
        if ext.android_ndk is None or ((not os.path.exists(os.path.join(ext.android_ndk, "ndk-build.cmd"))) and (not os.path.exists(os.path.join(ext.android_ndk, "ndk-build")))):
            self.errorMessage("Cannot find Android NDK")
            return False
        return super().checkGenerate(ext)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        cmake_android_arch_dict = {
            'arm':{
                'CMAKE_ANDROID_ARCH_ABI':'armeabi-v7a',
                'CMAKE_ANDROID_ARM_NEON':'on'
            },
            'arm64':{
                'CMAKE_ANDROID_ARCH_ABI':'arm64-v8a'
            },
            'x64':{
                'CMAKE_ANDROID_ARCH_ABI':'x86_64'
            },
            'x86':{
                'CMAKE_ANDROID_ARCH_ABI':'x86'
            }
        }

        cmd.addDict(cmake_android_arch_dict[self.getArchitecture()])
        cmd.addDict({
            "CMAKE_ANDROID_STL_TYPE":"c++_static",
            "CMAKE_SYSTEM_NAME":"Android",
            "CMAKE_SYSTEM_VERSION":28,
            "CMAKE_ANDROID_NDK":_norm_cmake_path(ext.android_ndk)
        })

class TizenCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("tizen", architecture, lto)
    def checkGenerate(self, ext):
        if ext.tizen_sdk is None or not os.path.exists(os.path.join(ext.tizen_sdk, "sdk.version")):
            self.errorMessage("Cannot find Tizen SDK")
            return False
        return super().checkGenerate(ext)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        cmd.addDict({
            "CMAKE_SYSTEM_NAME":"Tizen",
            "CMAKE_TIZEN_SDK":_norm_cmake_path(ext.tizen_sdk),
            "CMAKE_TIZEN_BUILD_TOOL": "clang",
            "CMAKE_TIZEN_SDK_VERSION": "7.0",
            "CMAKE_TIZEN_SDK_PLATFORM": "mobile"
        })
        if self.getArchitecture() == "arm":
            cmd.add("CMAKE_SYSTEM_PROCESSOR", "arm")
        elif self.getArchitecture() == "x86":
            cmd.add("CMAKE_SYSTEM_PROCESSOR", "i586")

class NSCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("ns", architecture, lto)
    def checkGenerate(self, ext):
        if ext.switch_sdk is None or not os.path.exists(os.path.join(ext.switch_sdk, "NintendoSdkRootMark")):
            self.errorMessage("Cannot find Nintendo Switch SDK")
            return False
        return super().checkGenerate(ext)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        cmd.addDict({
            "CMAKE_SYSTEM_NAME":"NintendoSwitch",
            "CMAKE_NINTENDO_SWITCH_SDK": _norm_cmake_path(ext.switch_sdk)
        })

class iOSCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("ios", architecture, lto)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        if ext.ios_dev_team is not None:
            cmd.add("CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM", ext.ios_dev_team)
        else:
            cmd.add("CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM", "iPhone Developer")
        cmd.add("CMAKE_SYSTEM_NAME", "iOS")
        cmd.add("CMAKE_OSX_ARCHITECTURES", "arm64")
        cmd.add("CMAKE_OSX_SYSROOT", "iphoneos")
    def configBuildCommand(self, build_path:str, cmd: List[str]):
        xcodeproj = list(filter(lambda x: x.endswith(".xcodeproj"), os.listdir(build_path)))
        if len(xcodeproj) != 0:
            cmd.extend(["--", "-allowProvisioningUpdates"])

class WindowsCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("windows", architecture, lto)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        vcvarsall = _find_vcvarsall(generator)
        cmd.addPrefixCommand([vcvarsall, _get_vcvarsall_arch(self.getArchitecture()), "&"])
        if generator != "ninja":
            #cmd.externalToken("-T").externalToken("ClangCl")
            if (self.getArchitecture() == "x64"):
                cmd.externalToken("-A").externalToken("x64")
            elif (self.getArchitecture() == "x86"):
                cmd.externalToken("-A").externalToken("Win32")
    def configBuildCommand(self, build_path:str, cmd: List[str]):
        vcvarsall = _find_vcvarsall("vs2022")
        cmd.insert(0, vcvarsall)
        cmd.insert(1, _get_vcvarsall_arch(self.getArchitecture()))
        cmd.insert(2, "&")
        
class LinuxCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("linux", architecture, lto)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        if (self.getArchitecture() == "x64"):
            cmd.add("CMAKE_C_FLAGS", "-m64").add("CMAKE_CXX_FLAGS", "-m64").add("CMAKE_ASM_FLAGS", "-m64")
        elif (self.getArchitecture() == "x86"):
            cmd.add("CMAKE_C_FLAGS", "-m32").add("CMAKE_CXX_FLAGS", "-m32").add("CMAKE_ASM_FLAGS", "-m32")

class macOSCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("macos", architecture, lto)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        if (self.getArchitecture() == "x64"):
            sys_processor = "x86_64"
        elif (self.getArchitecture() == "arm64"):
            sys_processor = "arm64"
        else:
            sys_processor = self.getArchitecture()
        cmd.add("CMAKE_OSX_ARCHITECTURES", sys_processor)

class EmscriptenCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str):
        super().__init__("emscripten", architecture, True)
        self._isWasm64 = architecture == "wasm64"
    def checkGenerate(self, ext):
        if ext.emscripten is None:
            self.errorMessage("Cannot find Emscripten")
            return False
        return super().checkGenerate(ext)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        if self.getArchitecture() == "wasm32":
            architecture = "x86"
        elif self.getArchitecture() == "wasm64":
            architecture = "x86_64"
        cmd.add("CMAKE_SYSTEM_NAME", "Emscripten")
        cmd.add("CMAKE_SYSTEM_PROCESSOR", architecture)
        cmd.add("CMAKE_TOOLCHAIN_FILE", _norm_cmake_path(os.path.join(ext.emscripten, "cmake", "Modules", "Platform", "Emscripten.cmake")))
    def beforeGenerate(self):
        if self._isWasm64:
            os.environ["CFLAGS"] = "-s MEMORY64"
    def afterGenerate(self):
        if self._isWasm64:
            os.environ["CFLAGS"] = ""

class UWPCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("uwp", architecture, lto)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        vcvarsall = _find_vcvarsall(generator)
        cmd.addPrefixCommand([vcvarsall, _get_vcvarsall_arch(self.getArchitecture()), "uwp", "&"])
        if generator != "ninja":
            if self.getArchitecture() == "x64":
                cmd.externalToken("-A").externalToken("x64")
            elif self.getArchitecture() == "x86":
                cmd.externalToken("-A").externalToken("Win32")
            elif self.getArchitecture() == "arm64":
                cmd.externalToken("-A").externalToken("ARM64")
            elif self.getArchitecture() == "arm":
                cmd.externalToken("-A").externalToken("ARM")
            cmd.add("CMAKE_SYSTEM_NAME", "WindowsStore")
            cmd.add("CMAKE_SYSTEM_VERSION", "10.0")
    def configBuildCommand(self, build_path:str, cmd: List[str]):
        vcvarsall = _find_vcvarsall("vs2022")
        cmd.insert(0, vcvarsall)
        cmd.insert(1, _get_vcvarsall_arch(self.getArchitecture()))
        cmd.insert(2, "uwp")
        cmd.insert(3, "&")

class AVRCommandGenerator(CMakeMicroControllerCommandGenerator):
    def __init__(self, mcu):
        super().__init__("avr", mcu)
    def checkGenerate(self, ext):
        if self.getMCU() is None:
            self.errorMessage("Please set avr mcu")
            return False
        return super().checkGenerate(ext)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        f_cpu = {
            "atmega32u4": "16000000",
            "atmega16u2": "16000000",
        }
        cmd.add("CMAKE_AVR_MCU", self.getMCU())  \
            .add("CMAKE_C_COMPILER", "avr-gcc")  \
            .add("CMAKE_CXX_COMPILER", "avr-g++")\
            .add("CMAKE_SYSTEM_NAME", "AVR")     \
            .add("CMAKE_AVR_F_CPU", f_cpu[self.getMCU()])

class PicoCommandGenerator(CMakeMicroControllerCommandGenerator):
    def __init__(self, mcu):
        super().__init__("pico", mcu)
    def checkGenerate(self, ext):
        if self.getMCU() is None:
            self.errorMessage("Please set pico mcu")
            return False
        return super().checkGenerate(ext)
    def configGenerateCommand(self, cmd: CMakeCommand, generator: str, ext: GenerateOptions):
        self.errorMessage("Not support pico now")

class FreeBSDCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("freebsd", architecture, lto)

class FuchsiaCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("fuchsia", architecture, lto)

class PS4CommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("ps4", architecture, lto)

class XboxOneCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("x1", architecture, lto)

class PS5CommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("ps5", architecture, lto)

class XboxSeriesCommandGenerator(CMakeCommandGenerator):
    def __init__(self, architecture: str, lto: bool):
        super().__init__("xsx", architecture, lto)

def _create_command_generator(platform: str, architecture: str, lto: bool, mcu: str):
    if (platform == "android"):
        return AndroidCommandGenerator(architecture, lto)
    elif (platform == "tizen"):
        return TizenCommandGenerator(architecture, lto)
    elif (platform == "ns"):
        return NSCommandGenerator(architecture, lto)
    elif (platform == "ios"):
        return iOSCommandGenerator(architecture, lto)
    elif (platform == "windows"):
        return WindowsCommandGenerator(architecture, lto)
    elif (platform == "linux"):
        return LinuxCommandGenerator(architecture, lto)
    elif (platform == "macos"):
        return macOSCommandGenerator(architecture, lto)
    elif (platform == "emscripten"):
        return EmscriptenCommandGenerator(architecture)
    elif (platform == "uwp"):
        return UWPCommandGenerator(architecture, lto)
    elif (platform == "avr"):
        return AVRCommandGenerator(mcu)
    elif (platform == "pico"):
        return PicoCommandGenerator(mcu)
    else:
        return None

def _find_util_root(binaryPath):
    if not os.path.exists(binaryPath):
        return None
    host_platform = _get_host_platform()
    host_arch = _get_host_architecture()
    if host_platform == "macos" and host_arch == "arm64":
        dir = host_platform + "-"
    else:
        dir = host_platform + "-" + host_arch
    platformDirs = []
    for i in reversed(os.listdir(binaryPath)):
        if i.startswith(dir):
            platformDirs.append(os.path.abspath(os.path.join(binaryPath, i)))
    
    for platformDir in platformDirs:
        fullPath = os.path.join(platformDir, "RuXPackageUtilsTargets.cmake")
        if os.path.exists(fullPath):
            return platformDir

class GenerateContext:
    def __init__(self, commandGenerator: CMakeCommandGenerator, buildDir):
        self._commandGenerator = commandGenerator
        self._buildDir = buildDir
        self._packagePath = ""

    def getPlatform(self):
        return self._commandGenerator.getPlatform()
    def getLto(self):
        return self._commandGenerator.getLto()
    def getArchitecture(self):
        return self._commandGenerator.getArchitecture()
    def getTriple(self):
        return self._commandGenerator.getTriple()
    def getMCU(self):
        if isinstance(self._commandGenerator, CMakeMicroControllerCommandGenerator):
            return self._commandGenerator.getMCU()
        return None

    def switchPackagePath(self, path):
        self._packagePath = path

    def getPackagePath(self):
        return self._packagePath

    def getBuildDir(self):
        return os.path.join(self._buildDir, self._commandGenerator.getTriple())

    def joinPaths(self, *arguments):
        return _norm_cmake_path(os.path.join(*arguments))

    def getUtilsRoot(self):
        return _find_util_root(self._buildDir)

class GenerateAction:
    def __init__(self, args, sourcePath = None):
        self._Args = args
        self._SrcPath = sourcePath
        self._Parser = argparse.ArgumentParser()
        self._PackageOptions = []
        self._Parser.add_argument('--lto', dest='lto', action='store_true', default=False)

        host_platform = _get_host_platform()
        self._Parser.add_argument('-p', '--platform', dest='platform', default=host_platform, choices=supportPlatform, help='Target platform')
        self._Parser.add_argument('-a', '--architecture', dest='architecture', default='default', choices=supportArchitecture, help='Target architecture')
        self._Parser.add_argument('-g', '--generator', dest='generator', default='default', choices=supportGenerator, help='CMake generator')
        self._Parser.add_argument('--mcu', dest='mcu', help='')
    
        self._Parser.add_argument('--android-ndk', dest='android_ndk', help='Android NDK Path')
        self._Parser.add_argument('--emscripten', dest='emscripten', help='Emscripten Path')
        self._Parser.add_argument('--switch-sdk', dest='switch_sdk', help='Nintendo Switch SDK Path')
        self._Parser.add_argument('--tizen-sdk', dest='tizen_sdk', help='Tizen SDK Path')
        self._Parser.add_argument('--ios-dev-team', dest='ios_dev_team', help='')

        packagesPath = os.path.join(self.getSourcePath(), "packages")
        thirdPartyPath = os.path.join(self.getSourcePath(), "third_party")

        packages = [ os.path.join(packagesPath, x) for x in os.listdir(packagesPath) ]
        packages += [ os.path.join(thirdPartyPath, x) for x in os.listdir(thirdPartyPath) ]
        packages = list(filter(lambda x: os.path.isdir(x), packages))
        for i in packages:
            if os.path.exists(os.path.join(i, "CMakeLists.txt")) and \
               os.path.exists(os.path.join(i, "RupkgInfo.cmake")) and \
               os.path.exists(os.path.join(i, "rupkg.py")):
                module_name = "__rupkg_desc__." + os.path.basename(i)
                rupkgDescSpec = importlib.util.spec_from_file_location(module_name, os.path.join(i, "rupkg.py"))
                rupkgDescModule = importlib.util.module_from_spec(rupkgDescSpec)
                rupkgDescSpec.loader.exec_module(rupkgDescModule)
                
                if rupkgDescModule.config_options is not None and rupkgDescModule.process_options is not None:
                    self._PackageOptions.append({ "path": i, "module": rupkgDescModule })
                    libraryOptions = self._Parser.add_argument_group(os.path.basename(i) + " Package Options")
                    rupkgDescModule.config_options(libraryOptions)

    def getSourcePath(self):
        if self._SrcPath is None:
            return os.getcwd()
        return self._SrcPath

    def getBinaryPath(self):
        return os.path.join(self.getSourcePath(), "build")

    def generate(self):
        options = self._Parser.parse_args(self._Args)

        commandGenerator = _create_command_generator(options.platform, options.architecture, options.lto, options.mcu)
        
        _host_platform = _get_host_platform()
        if not commandGenerator.getPlatform() in supportPlatformOnHost[_host_platform]:
            _print_error('%s cannot be built on %s' % (commandGenerator.getPlatform(), _host_platform))
            return -1

        ext = GenerateOptions()
        ext.android_ndk = options.android_ndk
        ext.emscripten = options.emscripten
        ext.switch_sdk = options.switch_sdk
        ext.tizen_sdk = options.tizen_sdk
        ext.ios_dev_team = options.ios_dev_team
        build_command = commandGenerator.createGenerateCommand(self.getSourcePath(), self.getBinaryPath(), options.generator, ext)

        context = GenerateContext(commandGenerator, self.getBinaryPath())

        for pkg in self._PackageOptions:
            context.switchPackagePath(os.path.relpath(pkg["path"], self.getSourcePath()))
            pkg["module"].process_options(options, build_command, context)

        if commandGenerator.getPlatform() != _get_host_platform():
            util_root = _find_util_root(self.getBinaryPath())
            if util_root is not None:
                build_command.add("RUPKG_UTIL_ROOT", util_root)

        build_command.add("CMAKE_CONFIGURATION_TYPES", ';'.join(supportConfiguration))
        build_command.add("CMAKE_EXPORT_COMPILE_COMMANDS", "on")

        if not os.path.exists(self.getBinaryPath()):
            os.makedirs(self.getBinaryPath())

        command = build_command.generateCommand("cmake")
        commandGenerator.beforeGenerate()
        result = subprocess.call(command)
        with open(os.path.join(build_command.getBinaryDir(), "CMakeCommands.txt"), "w") as f:
            f.writelines("\n".join(command))
        commandGenerator.afterGenerate()
        if result != 0:
            _print_error("Generate project failed")
            return -1
        return 0

class BuildAction:
    def __init__(self, args, binaryPath = None):
        self._Args = args
        self._DstPath = binaryPath
        self._Parser = argparse.ArgumentParser()
        self._Parser.add_argument('--lto', dest='lto', action='store_true', default=False)
        self._Parser.add_argument('-c', '--config', dest='config', nargs='*', default=['Release', 'Debug'],
            choices=supportConfiguration,
            help='configuration to deploy')

        self._Parser.add_argument('-t', '--target', dest='target', nargs='*', default=[])

        host_platform = _get_host_platform()
        self._Parser.add_argument('-p', '--platform', dest='platform', default=host_platform,
            choices=supportPlatform, help='Target platform')
        self._Parser.add_argument('-a', '--architecture', dest='architecture', default='default',
            choices=supportArchitecture, help='Target architecture')
        self._Parser.add_argument('--mcu', dest='mcu', help='')

        self._Parser.add_argument('-j', '--parallel', dest='parallel', default=min(multiprocessing.cpu_count() // 2, 1), help='Parallel build')

    def getBinaryPath(self):
        if self._DstPath is None:
            return os.path.join(os.getcwd(), "build")
        return self._DstPath

    def build(self):
        option = self._Parser.parse_args(self._Args)
        commandGenerator = _create_command_generator(option.platform, option.architecture, option.lto, option.mcu)
        config = option.config
        parallel = option.parallel

        build_command = commandGenerator.createBuildCommand(self.getBinaryPath(), config, parallel, option.target)
        for cmd in build_command:
            result = subprocess.call(cmd)
            if result != 0:
                _print_error("Build project failed")
                return -1
        return 0

def Build(args):
    action = BuildAction(args)
    return action.build()

def Generate(args):
    action = GenerateAction(args)
    return action.generate()

def Deploy(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', dest='output', default='./build/deploy/', help='destionation directory')
    parser.add_argument('-c', '--config', dest='config', nargs='*', default=supportConfiguration,
        choices=supportConfiguration,
        help='configuration to deploy')
    options = parser.parse_args(args)
    
    path = os.path.abspath(options.output)
    if (os.path.exists(path)):
        shutil.rmtree(path)

    for i in os.listdir(os.path.join(os.path.abspath("."), "build")):
        p = os.path.abspath(os.path.join(os.path.abspath("."), "build", i))
        if i != "deploy" and os.path.isdir(p):
            for c in options.config:
                if "linux" in i:
                    subprocess.call(["cmake", "--install", p, "--prefix", options.output, "--config", c, "--strip"])
                    continue
                subprocess.call(["cmake", "--install", p, "--prefix", options.output, "--config", c])
    
    return 0
    
def Main():
    if (len(sys.argv) < 2):
        print ('Select action: build, deploy, clean, generate')
        sys.exit()

    argv = sys.argv[2:]
    action = sys.argv[1]
    result = 0
    if (action == 'build'):
        result = Build(argv)
    elif (action == 'clean'):
        shutil.rmtree('./build')
        result = 0
    elif (action == 'generate'):
        result = Generate(argv)
    elif (action == 'deploy'):
        result = Deploy(argv)
    else:
        print ('Select action: build, deploy, clean, generate')
    sys.exit(result)

Main()
