import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent
    visible: false
    z: 999
    
    property Item blurTarget: null
    
    Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutExpo } }
    opacity: visible ? 1.0 : 0.0
    
    // Smooth fade backdrop
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.7)
        MouseArea { 
            anchors.fill: parent
            onClicked: root.visible = false 
        }
    }
    
    GlassPanel {
        width: 420
        height: 380
        anchors.centerIn: parent
        radius: 24
        blurSource: root.blurTarget
        
        MouseArea { anchors.fill: parent } // Prevent click-through
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 36
            spacing: 24
            
            ColumnLayout {
                spacing: 4
                Text {
                    text: "PREFERENCES"
                    color: theme.accentColor
                    font.pixelSize: 11
                    font.letterSpacing: 2
                    font.weight: Font.Bold
                }
                Text {
                    text: "System Settings"
                    color: theme.textColor
                    font.pixelSize: 24
                    font.weight: Font.Light
                }
            }
            
            Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 24
                Text {
                    text: "Visual Theme"
                    color: theme.subTextColor
                    font.pixelSize: 14
                    Layout.fillWidth: true
                }
                
                ComboBox {
                    id: themeCombo
                    model: ["Deep Space", "Alabaster", "Neon Obsidian"]
                    Layout.preferredWidth: 160
                    font.pixelSize: 13
                    onCurrentIndexChanged: {
                        if (currentIndex === 0) theme.setDarkTheme()
                        else if (currentIndex === 1) theme.setLightTheme()
                        else if (currentIndex === 2) theme.setCyberTheme()
                    }
                    
                    background: Rectangle {
                        color: theme.buttonBg
                        border.color: theme.panelBorder
                        border.width: 1
                        radius: 10
                    }
                    contentItem: Text {
                        text: themeCombo.displayText
                        font: themeCombo.font
                        color: theme.textColor
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        leftPadding: 12
                    }
                }
            }
            
            Item { Layout.fillHeight: true }
            
            Button {
                Layout.alignment: Qt.AlignRight
                text: "Dismiss"
                onClicked: root.visible = false
                contentItem: Text {
                    text: parent.text
                    color: theme.textColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }
                background: Rectangle {
                    implicitWidth: 110
                    implicitHeight: 40
                    radius: 12
                    color: parent.down ? theme.buttonDown : (parent.hovered ? theme.buttonHover : theme.buttonBg)
                    border.color: parent.hovered ? theme.panelBorder : "transparent"
                    border.width: 1
                    
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }
        }
    }
}
