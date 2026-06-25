# =============================================================================
# Arimu framework — minimal example build.
#
# Doubles as the canonical template for "how to wire the framework into a game".
# The three framework lines are marked  # <-- ARIMU.  Everything else is the
# standard Siv3D 0.6.16 link setup (lifted from the Arimu game project).
#
# Build (Windows):
#   cmake -S . -B build
#   cmake --build build --config Debug
# Needs Siv3D at C:/Siv3D_0.6.16 or env var SIV3D_0_6_16 (or -DSIV3D_DIR=...).
# =============================================================================
cmake_minimum_required(VERSION 3.24)
project(ArimuMinimal LANGUAGES CXX)

# The framework needs only C++20, but Siv3D 0.6.16 on recent MSVC toolchains
# pulls std::byteswap (C++23) from its headers — so prefer C++23 when available
# and match whatever standard your Siv3D build uses.
if("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(CMAKE_CXX_STANDARD 23)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_executable(ArimuMinimal WIN32 MACOSX_BUNDLE Main.cpp)

# --- ARIMU: pull in the framework (just two lines) --------------------------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. arimu-build)  # <-- ARIMU
target_link_libraries(ArimuMinimal PRIVATE arimu)                # <-- ARIMU (lib + include dirs + C++20)
# ----------------------------------------------------------------------------

if(MSVC)
    target_compile_options(ArimuMinimal PRIVATE /utf-8 /Zc:__cplusplus /EHsc)
    target_compile_definitions(ArimuMinimal PRIVATE
        _ENABLE_EXTENDED_ALIGNED_STORAGE
        _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)
endif()

# --- Siv3D 0.6.16 ------------------------------------------------------------
if(WIN32)
    if(NOT SIV3D_DIR)
        if(EXISTS "C:/Siv3D_0.6.16/include/Siv3D.hpp")
            set(SIV3D_DIR "C:/Siv3D_0.6.16")
        elseif(DEFINED ENV{SIV3D_0_6_16})
            set(SIV3D_DIR "$ENV{SIV3D_0_6_16}")
        endif()
    endif()
    if(NOT SIV3D_DIR OR NOT EXISTS "${SIV3D_DIR}")
        message(FATAL_ERROR "Siv3D not found. Set SIV3D_0_6_16 or pass -DSIV3D_DIR=...")
    endif()

    target_include_directories(ArimuMinimal PRIVATE
        "${SIV3D_DIR}/include" "${SIV3D_DIR}/include/ThirdParty")
    target_link_libraries(ArimuMinimal PRIVATE
        "$<$<CONFIG:Debug>:${SIV3D_DIR}/lib/Windows/Siv3D_d.lib>"
        "$<$<NOT:$<CONFIG:Debug>>:${SIV3D_DIR}/lib/Windows/Siv3D.lib>")
    target_link_directories(ArimuMinimal PRIVATE "${SIV3D_DIR}/lib/Windows")

    set(_delay advapi32.dll crypt32.dll dwmapi.dll gdi32.dll imm32.dll ole32.dll
        oleaut32.dll opengl32.dll shell32.dll shlwapi.dll user32.dll winmm.dll ws2_32.dll)
    foreach(_d IN LISTS _delay)
        target_link_options(ArimuMinimal PRIVATE "/DELAYLOAD:${_d}")
    endforeach()
    target_link_libraries(ArimuMinimal PRIVATE delayimp.lib)
elseif(APPLE)
    if(SIV3D_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${SIV3D_DIR}")
    endif()
    find_package(Siv3D REQUIRED)
    target_link_libraries(ArimuMinimal PRIVATE Siv3D::Siv3D)
endif()
