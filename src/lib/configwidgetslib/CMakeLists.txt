# Modified by Fcitx5 Fusion contributors, 2026-10-20: the retained Windows settings subset.
# Dropped with their features: addonselector (the addon list page, needs KF6
# KWidgetItemDelegate), layoutselector (the XKB keyboard-layout picker, needs layoutlib/X11),
# impage (input-method selection, a separate unimplemented surface).
add_library(configwidgetslib STATIC
    configwidget.cpp
    categoryhelper.cpp
    erroroverlay.cpp
    fontbutton.cpp
    keylistwidget.cpp
    listoptionwidget.cpp
    optionwidget.cpp
    optionaloptionwidget.cpp
    varianthelper.cpp
    verticalscrollarea.cpp
)
set_target_properties(configwidgetslib PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_include_directories(configwidgetslib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(configwidgetslib
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Widgets
    Fcitx5::Utils
    Fcitx5Qt${QT_MAJOR_VERSION}::DBusAddons
    Fcitx5Qt${QT_MAJOR_VERSION}::WidgetsAddons
    configlib)
