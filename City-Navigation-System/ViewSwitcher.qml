import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    width: 260
    height: 48

    property int currentIndex: 0 // 0: Controls, 1: Details
    property bool detailsEnabled: false
    signal switched(int index)

    GlassPanel {
        anchors.fill: parent
        radius: 14
        blurSource: parent.parent // Usually mapContainer
        
        // Darker tint for the switcher background
        Rectangle {
            anchors.fill: parent
            radius: 14
            color: Qt.rgba(0, 0, 0, 0.2)
            border.color: theme.panelBorder
            border.width: 1
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 4
            spacing: 4

            // Controls Tab
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: root.currentIndex === 0 ? theme.activePill : "transparent"
                border.color: root.currentIndex === 0 ? Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 0.3) : "transparent"
                border.width: 1
                
                Text {
                    anchors.centerIn: parent
                    text: i18n.t("viewswitcher.controls")
                    color: root.currentIndex === 0 ? theme.accentColor : theme.subTextColor
                    font.pixelSize: 13
                    font.weight: root.currentIndex === 0 ? Font.Bold : Font.Medium
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.switched(0)
                }
                
                Behavior on color { ColorAnimation { duration: 200 } }
            }

            // Details Tab
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: root.currentIndex === 1 ? theme.activePill : "transparent"
                border.color: root.currentIndex === 1 ? Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 0.3) : "transparent"
                border.width: 1
                opacity: root.detailsEnabled ? 1.0 : 0.3
                
                Text {
                    anchors.centerIn: parent
                    text: i18n.t("viewswitcher.details")
                    color: root.currentIndex === 1 ? theme.accentColor : theme.subTextColor
                    font.pixelSize: 13
                    font.weight: root.currentIndex === 1 ? Font.Bold : Font.Medium
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: root.detailsEnabled
                    cursorShape: root.detailsEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: root.switched(1)
                }
                
                Behavior on color { ColorAnimation { duration: 200 } }
                Behavior on opacity { NumberAnimation { duration: 200 } }
            }
        }
    }
}
