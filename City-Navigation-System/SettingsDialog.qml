import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

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
        height: 440
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
                    text: i18n.t("settings.header")
                    color: theme.accentColor
                    font.pixelSize: 11
                    font.letterSpacing: 2
                    font.weight: Font.Bold
                }
                Text {
                    text: i18n.t("settings.title")
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
                    text: i18n.t("settings.theme")
                    color: theme.subTextColor
                    font.pixelSize: 14
                    Layout.fillWidth: true
                }
                
                ComboBox {
                    id: themeCombo
                    model: i18n.themeModel
                    Layout.preferredWidth: 180
                    font.pixelSize: 13
                    onCurrentIndexChanged: {
                        if (currentIndex === 0) theme.setDarkTheme()
                        else if (currentIndex === 1) theme.setLightTheme()
                        else if (currentIndex === 2) theme.setCyberTheme()
                    }
                    
                    delegate: ItemDelegate {
                        width: themeCombo.width - 8
                        x: 4
                        height: 40
                        contentItem: Text {
                            text: modelData
                            color: highlighted ? theme.accentColor : theme.textColor
                            font.pixelSize: 13
                            font.weight: highlighted ? Font.Medium : Font.Normal
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                        background: Rectangle {
                            radius: 8
                            color: highlighted ? theme.activePill : "transparent"
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        highlighted: themeCombo.highlightedIndex === index
                    }

                    indicator: Canvas {
                        id: canvas
                        x: themeCombo.width - width - 12
                        y: (themeCombo.height - height) / 2
                        width: 10
                        height: 6
                        contextType: "2d"
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.moveTo(0, 0);
                            ctx.lineTo(width, 0);
                            ctx.lineTo(width / 2, height);
                            ctx.closePath();
                            ctx.fillStyle = theme.subTextColor;
                            ctx.fill();
                        }
                    }

                    background: Rectangle {
                        implicitHeight: 40
                        color: theme.buttonBg
                        border.color: themeCombo.visualFocus ? theme.accentColor : theme.panelBorder
                        border.width: 1
                        radius: 10
                        Behavior on border.color { ColorAnimation { duration: 200 } }
                    }

                    contentItem: Text {
                        text: themeCombo.displayText
                        font: themeCombo.font
                        color: theme.textColor
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        leftPadding: 16
                        rightPadding: 32
                    }

                    popup: Popup {
                        y: themeCombo.height + 6
                        width: themeCombo.width
                        implicitHeight: contentItem.implicitHeight + 16
                        padding: 8

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: themeCombo.popup.visible ? themeCombo.delegateModel : null
                            currentIndex: themeCombo.highlightedIndex
                            ScrollIndicator.vertical: ScrollIndicator { }
                        }

                        background: Rectangle {
                            color: Qt.rgba(theme.panelColor.r, theme.panelColor.g, theme.panelColor.b, 0.95)
                            border.color: theme.panelBorder
                            border.width: 1
                            radius: 16
                            
                            // Simple drop shadow effect
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                shadowEnabled: true
                                shadowBlur: 0.8
                                shadowColor: Qt.rgba(0, 0, 0, 0.4)
                                shadowVerticalOffset: 8
                            }
                        }
                        
                        enter: Transition {
                            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutCubic }
                            NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: 200; easing.type: Easing.OutBack }
                        }
                        exit: Transition {
                            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 150; easing.type: Easing.InCubic }
                            NumberAnimation { property: "scale"; from: 1.0; to: 0.95; duration: 150; easing.type: Easing.InCubic }
                        }
                    }
                }
            }
            
            Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor }

            RowLayout {
                Layout.fillWidth: true
                spacing: 24
                Text {
                    text: i18n.t("settings.language")
                    color: theme.subTextColor
                    font.pixelSize: 14
                    Layout.fillWidth: true
                }

                ComboBox {
                    id: langCombo
                    model: i18n.langModel
                    Layout.preferredWidth: 180
                    font.pixelSize: 13
                    currentIndex: i18n.language === "zh" ? 0 : 1
                    onCurrentIndexChanged: {
                        i18n.language = currentIndex === 0 ? "zh" : "en"
                    }

                    delegate: ItemDelegate {
                        width: langCombo.width - 8
                        x: 4
                        height: 40
                        contentItem: Text {
                            text: modelData
                            color: highlighted ? theme.accentColor : theme.textColor
                            font.pixelSize: 13
                            font.weight: highlighted ? Font.Medium : Font.Normal
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                        background: Rectangle {
                            radius: 8
                            color: highlighted ? theme.activePill : "transparent"
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        highlighted: langCombo.highlightedIndex === index
                    }

                    indicator: Canvas {
                        id: langCanvas
                        x: langCombo.width - width - 12
                        y: (langCombo.height - height) / 2
                        width: 10
                        height: 6
                        contextType: "2d"
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.moveTo(0, 0);
                            ctx.lineTo(width, 0);
                            ctx.lineTo(width / 2, height);
                            ctx.closePath();
                            ctx.fillStyle = theme.subTextColor;
                            ctx.fill();
                        }
                    }

                    background: Rectangle {
                        implicitHeight: 40
                        color: theme.buttonBg
                        border.color: langCombo.visualFocus ? theme.accentColor : theme.panelBorder
                        border.width: 1
                        radius: 10
                        Behavior on border.color { ColorAnimation { duration: 200 } }
                    }

                    contentItem: Text {
                        text: langCombo.displayText
                        font: langCombo.font
                        color: theme.textColor
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        leftPadding: 16
                        rightPadding: 32
                    }

                    popup: Popup {
                        y: langCombo.height + 6
                        width: langCombo.width
                        implicitHeight: contentItem.implicitHeight + 16
                        padding: 8

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: langCombo.popup.visible ? langCombo.delegateModel : null
                            currentIndex: langCombo.highlightedIndex
                            ScrollIndicator.vertical: ScrollIndicator { }
                        }

                        background: Rectangle {
                            color: Qt.rgba(theme.panelColor.r, theme.panelColor.g, theme.panelColor.b, 0.95)
                            border.color: theme.panelBorder
                            border.width: 1
                            radius: 16

                            layer.enabled: true
                            layer.effect: MultiEffect {
                                shadowEnabled: true
                                shadowBlur: 0.8
                                shadowColor: Qt.rgba(0, 0, 0, 0.4)
                                shadowVerticalOffset: 8
                            }
                        }

                        enter: Transition {
                            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutCubic }
                            NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: 200; easing.type: Easing.OutBack }
                        }
                        exit: Transition {
                            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 150; easing.type: Easing.InCubic }
                            NumberAnimation { property: "scale"; from: 1.0; to: 0.95; duration: 150; easing.type: Easing.InCubic }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }

            Button {
                Layout.alignment: Qt.AlignRight
                text: i18n.t("settings.dismiss")
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
