cmake_minimum_required(VERSION 3.21)
project(FleetManager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
	Core
	Gui
	Widgets
	Sql
	REQUIRED)

add_executable(FleetManager main.cpp)

target_link_libraries(FleetManager
	Qt::Core
	Qt::Gui
	Qt::Widgets
	Qt::Sql
)

