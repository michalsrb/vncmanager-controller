cmake_minimum_required(VERSION 2.8.11)

project(vncmanager-controller)

include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5X11Extras REQUIRED)

add_definitions(-std=c++11)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(vncmanager-controller_SRCS ConfigWindow.cpp StdinConnection.cpp VncConfiguration.cpp ManagerConnection.cpp main.cpp)

set(vncmanager-controller_FORMS ConfigWindow.ui)

qt5_wrap_ui(vncmanager-controller_FORMS_HEADERS ${vncmanager-controller_FORMS})

add_executable(vncmanager-controller ${vncmanager-controller_SRCS} ${vncmanager-controller_FORMS_HEADERS})
target_link_libraries(vncmanager-controller Qt5::Widgets Qt5::Network Qt5::X11Extras X11 Xvnc)

install(TARGETS vncmanager-controller RUNTIME DESTINATION bin)
install(FILES vncmanager-controller.desktop DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart)
install(FILES gnome-shell-extension/extension.js gnome-shell-extension/metadata.json DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gnome-shell/extensions/vncmanager-controller@vncmanager.michalsrb.github.com)
