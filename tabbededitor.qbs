import qbs 1.0

QtcPlugin {
    name: "TabbedEditor"

    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "Qt.widgets" }

    files: [
      "tabbededitorplugin.cpp",
      "tabbededitorplugin.h"
    ]
}

