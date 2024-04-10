# SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

project(torrent_view VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DEMO_VIEWER "build torrent viewer application" OFF)

set(USE_QT_MAJOR_VERSION "6" CACHE STRING "Major Qt version to use")
find_package(Qt${USE_QT_MAJOR_VERSION} REQUIRED COMPONENTS Widgets)
find_package(LibtorrentRasterbar 2.0 REQUIRED)

set(PROJECT_SOURCES
  torrentinfoview.cpp
  torrentinfoview.hpp
  torrentinfoview.ui
  torrentfilesmodel.cpp
  torrentfilesmodel.hpp
  torrenttrackersmodel.cpp
  torrenttrackersmodel.hpp
  utils.cpp
  utils.hpp
)

set(dep_libs Qt::Widgets LibtorrentRasterbar::torrent-rasterbar)

if (BUILD_DEMO_VIEWER)
  add_executable(torrent_view main.cpp ${PROJECT_SOURCES})
  target_link_libraries(torrent_view PRIVATE ${dep_libs})
endif()

set(plugin_target torrent_view_qt${USE_QT_MAJOR_VERSION})

add_library(${plugin_target} MODULE plugin.cpp ${PROJECT_SOURCES})
set_target_properties(${plugin_target} PROPERTIES PREFIX "" SUFFIX ".wlx")
target_include_directories(${plugin_target} PRIVATE sdk)
target_link_libraries(${plugin_target} PRIVATE ${dep_libs})
