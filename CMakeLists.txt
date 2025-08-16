cmake_minimum_required(VERSION 3.15)
project(SquadroAI_Cpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# مسیرهای include برای هدرهای خودمان
include_directories(include)

# استفاده از FetchContent برای دانلود و مدیریت کتابخانه‌های ثالث
include(FetchContent)

# # --- cpp-httplib ---
# set(CPPHTTPLIB_OPENSSL_SUPPORT OFF CACHE BOOL "Disable SSL for cpp-httplib if not needed")
# FetchContent_Declare(
#     cpp-httplib-fetch # یک نام منحصر به فرد برای این FetchContent_Declare
#     GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
#     GIT_TAG v0.15.3 # یا آخرین نسخه پایدار
# )
# FetchContent_GetProperties(cpp-httplib-fetch)
# if(NOT cpp-httplib-fetch_POPULATED) # چک کنید که آیا قبلاً دانلود شده یا نه
#     FetchContent_Populate(cpp-httplib-fetch)
#     # cpp-httplib یک کتابخانه header-only است، بنابراین فقط نیاز به افزودن مسیر include آن داریم
#     include_directories(${cpp-httplib-fetch_SOURCE_DIR})
#     message(STATUS "cpp-httplib source dir: ${cpp-httplib-fetch_SOURCE_DIR}")
# endif()

# # --- nlohmann-json ---
# FetchContent_Declare(
#     nlohmann_json-fetch # یک نام منحصر به فرد
#     GIT_REPOSITORY https://github.com/nlohmann/json.git
#     GIT_TAG v3.11.3 # یا آخرین نسخه پایدار
# )
# FetchContent_GetProperties(nlohmann_json-fetch)
# if(NOT nlohmann_json-fetch_POPULATED)
#     FetchContent_Populate(nlohmann_json-fetch)
#     # nlohmann/json هم header-only است و معمولاً در یک زیرپوشه 'include' قرار دارد
#     include_directories(${nlohmann_json-fetch_SOURCE_DIR}/include)
#     message(STATUS "nlohmann_json source dir: ${nlohmann_json-fetch_SOURCE_DIR}/include")
# endif()


# تعریف کتابخانه برای منطق بازی و هوش مصنوعی
add_library(squadro_ai_lib STATIC
    src/AIPlayer.cpp
    src/Board.cpp
    src/GameState.cpp
    src/GameStateHasher.cpp
    src/Heuristics.cpp
    src/NetworkManager.cpp
    src/TranspositionTable.cpp
)

# تعریف فایل اجرایی اصلی
add_executable(SquadroAI_App src/main.cpp)
add_executable(SquadroAI_Test src/test_ai.cpp)

# لینک کردن کتابخانه به فایل‌های اجرایی
target_link_libraries(SquadroAI_App PRIVATE squadro_ai_lib)
target_link_libraries(SquadroAI_Test PRIVATE squadro_ai_lib)

# اطمینان از اینکه هدرهای عمومی کتابخانه squadro_ai_lib برای SquadroAI_App قابل دسترس هستند
target_include_directories(squadro_ai_lib PUBLIC include)


# فعال کردن هشدارها
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(squadro_ai_lib INTERFACE -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough)
    target_compile_options(SquadroAI_App INTERFACE -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        target_compile_options(squadro_ai_lib INTERFACE -Wthread-safety) # اگر از thread safety analysis استفاده می‌کنید
        target_compile_options(SquadroAI_App INTERFACE -Wthread-safety)
    endif()
elseif(MSVC)
    target_compile_options(squadro_ai_lib INTERFACE /W4 /WX /permissive- /wd4100 /wd4189 /wd4505) # /WX برای برخورد با هشدارها به عنوان خطا
    target_compile_options(SquadroAI_App INTERFACE /W4 /WX /permissive- /wd4100 /wd4189 /wd4505)
endif()

# --- Advanced Compiler Optimizations ---
option(ENABLE_LTO "Enable Link-Time Optimization" ON)
option(ENABLE_NATIVE_ARCH "Enable CPU-specific optimizations for the host machine" ON)
option(USE_PGO_GENERATE "Build with PGO instrumentation for profiling" OFF)
option(USE_PGO_USE "Build with PGO profile data" OFF)

# تنظیمات برای build release با بهینه‌سازی
# این بهتر است خارج از اینجا و با پاس دادن -DCMAKE_BUILD_TYPE=Release به cmake انجام شود
# اما برای راحتی اینجا هم می‌گذاریم
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring for Release build")

    set(RELEASE_FLAGS "-O3 -DNDEBUG")
    if(ENABLE_LTO)
        set(RELEASE_FLAGS "${RELEASE_FLAGS} -flto")
        message(STATUS "LTO enabled")
    endif()
    if(ENABLE_NATIVE_ARCH)
        set(RELEASE_FLAGS "${RELEASE_FLAGS} -march=native")
        message(STATUS "Native architecture optimizations enabled")
    endif()
    if(USE_PGO_GENERATE)
        set(RELEASE_FLAGS "${RELEASE_FLAGS} -fprofile-generate")
        message(STATUS "PGO-Generate enabled")
    elseif(USE_PGO_USE)
        set(RELEASE_FLAGS "${RELEASE_FLAGS} -fprofile-use")
        message(STATUS "PGO-Use enabled")
    endif()

    if(MSVC)
        # MSVC flags would go here
    else()
        target_compile_options(squadro_ai_lib INTERFACE ${RELEASE_FLAGS})
        target_compile_options(SquadroAI_App INTERFACE ${RELEASE_FLAGS})
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for Debug build")
    if(MSVC)
        target_compile_options(squadro_ai_lib INTERFACE /Zi /Od)
        target_compile_options(SquadroAI_App INTERFACE /Zi /Od)
    else()
        target_compile_options(squadro_ai_lib INTERFACE -g -O0)
        target_compile_options(SquadroAI_App INTERFACE -g -O0)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "Configuring for Release with Debug Info build")
    if(MSVC)
        target_compile_options(squadro_ai_lib INTERFACE /Zi /O2 /Ob2 /DNDEBUG)
        target_compile_options(SquadroAI_App INTERFACE /Zi /O2 /Ob2 /DNDEBUG)
    else()
        target_compile_options(squadro_ai_lib INTERFACE -g -O2 -DNDEBUG)
        target_compile_options(SquadroAI_App INTERFACE -g -O2 -DNDEBUG)
    endif()
endif()

# اضافه کردن پشتیبانی از Thread ها (معمولا خودکار انجام می‌شود با C++11 به بالا)
# اما برای اطمینان، اگر کامپایلر قدیمی باشد:
# find_package(Threads REQUIRED)
# target_link_libraries(SquadroAI_App PRIVATE Threads::Threads)


message(STATUS "CMake configuration finished. Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "You can now build the project using your build system (e.g., make, ninja, Visual Studio).")
