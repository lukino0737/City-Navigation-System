import QtQuick
import QtQuick.Controls

Item {
    id: root
    height: 40

    property Window targetWindow: null
    property Item   blurSource: null

    property bool windowMaximized: false
    property rect savedGeometry

    function toggleMaximize() {
        if (!targetWindow) return
        if (windowMaximized) {
            targetWindow.visibility = Window.Windowed
            targetWindow.x = savedGeometry.x
            targetWindow.y = savedGeometry.y
            targetWindow.width = savedGeometry.width
            targetWindow.height = savedGeometry.height
            windowMaximized = false
        } else {
            savedGeometry = Qt.rect(targetWindow.x, targetWindow.y, targetWindow.width, targetWindow.height)
            windowMaximized = true
            targetWindow.visibility = Window.Maximized
        }
    }

    // ── Acrylic Glass Base ──────────────────────────────────────────────
    GlassPanel {
        anchors.fill: parent
        radius: 0
        blurSource: root.blurSource
        glassColor: theme.panelColor
        borderColor: "transparent"
    }

    // ── Bottom separator ─────────────────────────────────────────────────
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color: theme.dividerColor
    }

    // ── Drag-to-move region ──────────────────────────────────────────────
    MouseArea {
        id: dragRegion
        anchors.fill: parent
        anchors.rightMargin: 150
        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.ArrowCursor
        onPressed: {
            if (root.targetWindow)
                root.targetWindow.startSystemMove()
        }
        onDoubleClicked: root.toggleMaximize()
    }

    // ── App icon + title ─────────────────────────────────────────────────
    Row {
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        // Fluent-style diamond icon
        Item {
            width: 16; height: 16
            anchors.verticalCenter: parent.verticalCenter
            Rectangle {
                width: 10; height: 10
                anchors.centerIn: parent
                rotation: 45
                radius: 1.5
                color: theme.accentColor
                opacity: 0.75
            }
        }

        Text {
            text: i18n.t("app.title")
            color: theme.textColor
            font.pixelSize: 12
            font.weight: Font.DemiBold
            font.letterSpacing: 1.5
            opacity: 0.82
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    // ── Window Control Button ────────────────────────────────────────────
    component WinButton : Item {
        id: btn
        property bool   isClose: false
        property string icon:    "min"   // "min" | "max" | "restore" | "close"
        signal clicked()

        width: 48; height: 32

        // Background pill
        Rectangle {
            id: bg
            anchors.fill: parent
            radius: 8
            color: {
                if (btn.isClose && btnMA.containsMouse)
                    return btnMA.pressed ? "#a01c2a" : "#c42b1c"
                if (btnMA.containsMouse)
                    return btnMA.pressed ? theme.buttonDown : theme.buttonHover
                return "transparent"
            }
            Behavior on color { ColorAnimation { duration: 180; easing.type: Easing.OutCubic } }
        }

        // ── Minimize icon ────────────────────────────────────────────
        Rectangle {
            visible: btn.icon === "min"
            width: 10; height: 1.5
            anchors.centerIn: parent
            radius: 0.75
            color: btnMA.containsMouse ? theme.textColor : theme.subTextColor
            opacity: btnMA.containsMouse ? 1.0 : 0.6
            Behavior on color   { ColorAnimation { duration: 150 } }
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        // ── Maximize icon ────────────────────────────────────────────
        Rectangle {
            visible: btn.icon === "max"
            width: 10; height: 10
            anchors.centerIn: parent
            radius: 1.5
            color: "transparent"
            border.width: 1.5
            border.color: btnMA.containsMouse ? theme.textColor : theme.subTextColor
            opacity: btnMA.containsMouse ? 1.0 : 0.6
            Behavior on border.color { ColorAnimation { duration: 150 } }
            Behavior on opacity     { NumberAnimation { duration: 150 } }
        }

        // ── Restore icon ─────────────────────────────────────────────
        Item {
            visible: btn.icon === "restore"
            width: 10; height: 10
            anchors.centerIn: parent

            Rectangle {
                x: 2.5; y: 0
                width: 7.5; height: 7.5
                radius: 1.5
                color: "transparent"
                border.width: 1.5
                border.color: btnMA.containsMouse ? theme.textColor : theme.subTextColor
                opacity: btnMA.containsMouse ? 1.0 : 0.6
                Behavior on border.color { ColorAnimation { duration: 150 } }
                Behavior on opacity     { NumberAnimation { duration: 150 } }
            }
            Rectangle {
                x: 0; y: 2.5
                width: 7.5; height: 7.5
                radius: 1.5
                color: theme.panelColor
                border.width: 1.5
                border.color: btnMA.containsMouse ? theme.textColor : theme.subTextColor
                opacity: btnMA.containsMouse ? 1.0 : 0.6
                Behavior on border.color { ColorAnimation { duration: 150 } }
                Behavior on opacity     { NumberAnimation { duration: 150 } }
            }
        }

        // ── Close icon (cross) ───────────────────────────────────────
        Item {
            visible: btn.icon === "close"
            width: 10; height: 10
            anchors.centerIn: parent
            Rectangle {
                width: 10; height: 1.5
                anchors.centerIn: parent
                rotation: 45
                radius: 0.75
                color: btnMA.containsMouse ? "#ffffff" : theme.subTextColor
                opacity: btnMA.containsMouse ? 1.0 : 0.6
                Behavior on color   { ColorAnimation { duration: 150 } }
                Behavior on opacity { NumberAnimation { duration: 150 } }
            }
            Rectangle {
                width: 10; height: 1.5
                anchors.centerIn: parent
                rotation: -45
                radius: 0.75
                color: btnMA.containsMouse ? "#ffffff" : theme.subTextColor
                opacity: btnMA.containsMouse ? 1.0 : 0.6
                Behavior on color   { ColorAnimation { duration: 150 } }
                Behavior on opacity { NumberAnimation { duration: 150 } }
            }
        }

        MouseArea {
            id: btnMA
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: btn.clicked()
        }
    }

    // ── Window Control Row ───────────────────────────────────────────────
    Row {
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        WinButton {
            icon: "min"
            onClicked: {
                if (root.targetWindow)
                    root.targetWindow.visibility = Window.Minimized
            }
        }
        WinButton {
            icon: root.windowMaximized ? "restore" : "max"
            onClicked: root.toggleMaximize()
        }
        WinButton {
            icon: "close"
            isClose: true
            onClicked: {
                if (root.targetWindow)
                    root.targetWindow.close()
            }
        }
    }
}
