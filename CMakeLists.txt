cmake_minimum_required(VERSION 3.21)
project(FleetManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем поддержку автогенерации Qt MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Настройка директории для UIC
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)

# Поиск Qt6
find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Sql
    REQUIRED)

# Добавляем subdirectory с исходниками
add_subdirectory(src)
