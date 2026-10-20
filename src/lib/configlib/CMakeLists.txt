# Modified by Fcitx5 Fusion contributors, 2026-10-20. This Windows settings subset
# compiles the product-owned FusionHost transport into FusionSettings.exe and retains
# the upstream configtool helpers here.
# The addon list, input-method page, keyboard layout and migrator are excluded here.
# launchexternal.cpp keeps the upstream External-option helper linkable, while the
# Windows product disables External actions because its wrappers are not shipped.
# Generate config.h as the upstream top-level CMakeLists does for this pinned version.
set(PROJECT_VERSION 5.1.16)
set(FCITX5_QT_GUI_WRAPPER "fcitx5-qt6-gui-wrapper")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../../../config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/config.h")
add_library(configlib STATIC
    launchexternal.cpp
    logging.cpp
    font.cpp
)
set_target_properties(configlib PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
)
target_include_directories(configlib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(configlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(configlib
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Fcitx5Qt${QT_MAJOR_VERSION}::DBusAddons
    Fcitx5::Core
    Fcitx5::Utils
    Fcitx5Qt${QT_MAJOR_VERSION}::WidgetsAddons
)
