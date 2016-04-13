# Название проекта
project (FlappyUrho)
# Имя результируюшего исполняемого файла
set (TARGET_NAME FlappyUrho)
# Можно не использовать переменные окружения, а указать путь к скомпилированному движку в самом скрипте
set (ENV{URHO3D_HOME} D:/FlappyUrho/Engine/Build)
# Бывает удобно не копировать папку CMake в директорию с исходниками игры, а просто указать путь к ней
set (CMAKE_MODULE_PATH D:/FlappyUrho/Engine/Urho3D/CMake/Modules)

# Остальное менять не нужно
cmake_minimum_required (VERSION 2.8.6)
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
    if (CMAKE_VERSION VERSION_GREATER 2.8.12 OR CMAKE_VERSION VERSION_EQUAL 2.8.12)
        cmake_policy (SET CMP0022 NEW)
    endif ()
    if (CMAKE_VERSION VERSION_GREATER 3.0.0 OR CMAKE_VERSION VERSION_EQUAL 3.0.0)
        cmake_policy (SET CMP0026 OLD)
        cmake_policy (SET CMP0042 NEW)
    endif ()
endif ()
include (Urho3D-CMake-common)
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS})
define_source_files ()
setup_main_executable ()
