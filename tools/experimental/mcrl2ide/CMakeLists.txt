add_mcrl2_tool(mcrl2ide
  MENUNAME "mCRL2 IDE"
  DESCRIPTION "A graphical IDE for creating, analyzing, visualizing and verifying mCRL2 specifications"
  ICON "mcrl2-red"
  SOURCES
    addeditpropertydialog.cpp
    addeditpropertydialog.ui
    codeeditor.cpp
    consoledock.cpp
    filesystem.cpp
    findandreplacedialog.cpp
    findandreplacedialog.ui
    main.cpp
    mainwindow.cpp
    mcrl2-ide.qrc
    processsystem.cpp
    propertiesdock.cpp
    propertywidget.cpp
    rewritedock.cpp
    solvedock.cpp
  DEPENDS
    mcrl2_gui
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
)
