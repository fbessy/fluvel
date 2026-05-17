# =========================
# Qt configuration
# =========================

set(QT_COMPONENTS
    Core
    Gui
    Multimedia
    MultimediaWidgets
    Widgets
    Svg
    LinguistTools
)

if(FLUVEL_USE_QML)
    list(APPEND QT_COMPONENTS
        Qml
        Quick
    )
endif()

find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})

if(COMMAND qt_policy)
    qt_policy(SET QTP0002 NEW)
endif()