import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: 72

    property var mapView: null
    property Item blurTarget: null
    signal settingsClicked()

    // 阻止鼠标事件穿透到下方的地图
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
    }

    GlassPanel {
        anchors.fill: parent
        radius: 20
        blurSource: root.blurTarget
    }

    component SidebarBtn: Item {
        id: btnRoot
        property string iconText: ""
        property string tipText: ""
        property bool isActive: false
        signal clicked()

        width: 56
        height: 56
        anchors.horizontalCenter: parent.horizontalCenter

        // Premium selection pill
        Rectangle {
            anchors.centerIn: parent
            width: isActive ? 48 : (mouseArea.containsMouse ? 44 : 36)
            height: isActive ? 48 : (mouseArea.containsMouse ? 44 : 36)
            radius: 16
            color: isActive ? theme.activePill : (mouseArea.containsMouse ? theme.buttonHover : "transparent")
            border.width: 1
            border.color: isActive ? Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 0.4) : (mouseArea.containsMouse ? theme.panelBorder : "transparent")

            Behavior on width { NumberAnimation { duration: 350; easing.type: Easing.OutQuint } }
            Behavior on height { NumberAnimation { duration: 350; easing.type: Easing.OutQuint } }
            Behavior on color { ColorAnimation { duration: 250 } }
            Behavior on border.color { ColorAnimation { duration: 250 } }
        }

        Text {
            anchors.centerIn: parent
            text: btnRoot.iconText
            font.pixelSize: isActive ? 20 : 18
            color: isActive ? theme.accentColor : theme.subTextColor
            opacity: isActive ? 1.0 : (mouseArea.containsMouse ? 0.9 : 0.6)

            Behavior on font.pixelSize { NumberAnimation { duration: 350; easing.type: Easing.OutQuint } }
            Behavior on opacity { NumberAnimation { duration: 250 } }
            Behavior on color { ColorAnimation { duration: 250 } }
        }

        // Hover tooltip
        Rectangle {
            x: btnRoot.width + 8
            anchors.verticalCenter: parent.verticalCenter
            width: tipLabel.implicitWidth + 20
            height: 28
            radius: 6
            color: theme.panelColor
            border.color: theme.panelBorder
            border.width: 1
            visible: mouseArea.containsMouse && tipText !== ""
            z: 100

            Text {
                id: tipLabel
                anchors.centerIn: parent
                text: btnRoot.tipText
                color: theme.textColor
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            Behavior on opacity { NumberAnimation { duration: 150 } }
            opacity: visible ? 1.0 : 0.0
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: btnRoot.clicked()
            cursorShape: Qt.PointingHandCursor
        }
    }

    Column {
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 16
        
        SidebarBtn {
            iconText: "⌘"
            tipText: i18n.t("sidebar.view")
            isActive: !root.mapView || !root.mapView.routeMode
            onClicked: {
                if (root.mapView) root.mapView.routeMode = false
            }
        }
        SidebarBtn {
            visible: false
            iconText: "∿"
            tipText: i18n.t("sidebar.traffic")
            isActive: false
        }
        SidebarBtn {
            iconText: "↬"
            tipText: i18n.t("sidebar.route")
            isActive: root.mapView ? root.mapView.routeMode : false
            onClicked: {
                if (root.mapView) root.mapView.routeMode = !root.mapView.routeMode
            }
        }
    }

    Column {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter
        
        SidebarBtn {
            iconText: "⚲"
            tipText: i18n.t("sidebar.settings")
            onClicked: root.settingsClicked()
        }
    }
}
