import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    width: 340

    property bool isCollapsed: false
    property var mapView: null
    property Item blurTarget: null
    property bool isGenerating: false
    signal regenerateClicked()

    Behavior on x { NumberAnimation { duration: 450; easing.type: Easing.OutQuint } }

    x: isCollapsed ? parent.width : parent.width - width - 24

    // 阻止鼠标事件穿透到下方的地图
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
    }

    GlassPanel {
        id: mainGlass
        anchors.fill: parent
        radius: 24
        blurSource: root.blurTarget
    }

    // Floating collapse button
    Item {
        width: 32
        height: 64
        anchors.right: parent.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        
        GlassPanel {
            anchors.fill: parent
            radius: 12
            blurSource: root.blurTarget
        }
        
        Text {
            anchors.centerIn: parent
            text: root.isCollapsed ? "‹" : "›"
            color: theme.subTextColor
            font.pixelSize: 18
            font.weight: Font.Light
        }
        
        MouseArea {
            anchors.fill: parent
            onClicked: root.isCollapsed = !root.isCollapsed
            cursorShape: Qt.PointingHandCursor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 28

        ColumnLayout {
            spacing: 4
            Text {
                text: "NAVIGATION"
                color: theme.accentColor
                font.pixelSize: 11
                font.letterSpacing: 2
                font.weight: Font.Bold
            }
            Text {
                text: "Control Center"
                color: theme.textColor
                font.pixelSize: 24
                font.weight: Font.Light
            }
        }
        
        Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor }
        
        GridLayout {
            columns: 2
            rowSpacing: 16
            columnSpacing: 24
            Layout.fillWidth: true
            
            Text { text: "Total Nodes"; color: theme.subTextColor; font.pixelSize: 13 }
            Text { text: "10,000"; color: theme.textColor; font.pixelSize: 14; font.weight: Font.Medium; Layout.alignment: Qt.AlignRight }
            
            Text { text: "Zoom Level"; color: theme.subTextColor; font.pixelSize: 13 }
            Text { text: (mapView ? mapView.zoom.toFixed(2) : "1.0") + "x"; color: theme.textColor; font.pixelSize: 14; font.weight: Font.Medium; Layout.alignment: Qt.AlignRight }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor }
        
        ColumnLayout {
            spacing: 12
            Layout.fillWidth: true
            
            Button {
                id: regenBtn
                Layout.fillWidth: true
                text: root.isGenerating ? "Generating..." : "Regenerate Network"
                enabled: !root.isGenerating
                onClicked: {
                    if (!root.isGenerating) {
                        root.regenerateClicked()
                    }
                }
                contentItem: Text {
                    text: regenBtn.text
                    color: root.isGenerating ? theme.subTextColor : theme.textColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }
                background: Rectangle {
                    implicitHeight: 44
                    radius: 12
                    color: regenBtn.down ? theme.buttonDown : (regenBtn.hovered ? theme.buttonHover : theme.buttonBg)
                    border.color: regenBtn.hovered ? theme.panelBorder : "transparent"
                    border.width: 1
                }
            }

            Button {
                id: lodBtn
                Layout.fillWidth: true
                text: mapView && mapView.lodEnabled ? "LOD: Active" : "LOD: Inactive"
                onClicked: {
                    if (mapView) mapView.lodEnabled = !mapView.lodEnabled
                }
                contentItem: Text {
                    text: lodBtn.text
                    color: (mapView && mapView.lodEnabled) ? theme.accentColor : theme.subTextColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }
                background: Rectangle {
                    implicitHeight: 44
                    radius: 12
                    color: (mapView && mapView.lodEnabled) ? theme.activePill : (lodBtn.hovered ? theme.buttonHover : "transparent")
                    border.color: (mapView && mapView.lodEnabled) ? Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 0.4) : (lodBtn.hovered ? theme.panelBorder : "transparent")
                    border.width: 1
                }
            }

            // LOD Intensity Slider
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                opacity: (mapView && mapView.lodEnabled) ? 1.0 : 0.35
                Behavior on opacity { NumberAnimation { duration: 200 } }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "LOD Intensity"
                        color: theme.subTextColor
                        font.pixelSize: 12
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        id: lodValueLabel
                        // Map slider value (0–100) to label: Soft / Balanced / Aggressive
                        text: {
                            if (!mapView) return "–"
                            let v = mapView.lodIntensity
                            if (v < 0.9)  return "Soft (" + v.toFixed(1) + ")"
                            if (v < 2.1)  return "Balanced (" + v.toFixed(1) + ")"
                            return "Aggressive (" + v.toFixed(1) + ")"
                        }
                        color: theme.accentColor
                        font.pixelSize: 11
                        font.weight: Font.Medium
                    }
                }

                // Custom-styled Slider
                Item {
                    Layout.fillWidth: true
                    height: 24

                    // Track background
                    Rectangle {
                        id: trackBg
                        width: parent.width
                        height: 4
                        anchors.verticalCenter: parent.verticalCenter
                        radius: 2
                        color: theme.dividerColor
                    }

                    // Filled portion (left of handle)
                    Rectangle {
                        width: lodSlider.visualPosition * trackBg.width
                        height: 4
                        anchors.verticalCenter: parent.verticalCenter
                        radius: 2
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 0.5) }
                            GradientStop { position: 1.0; color: theme.accentColor }
                        }
                    }

                    Slider {
                        id: lodSlider
                        anchors.fill: parent
                        from: 0.3
                        to: 3.0
                        stepSize: 0.1
                        value: mapView ? mapView.lodIntensity : 1.5
                        enabled: mapView && mapView.lodEnabled

                        onMoved: {
                            if (mapView) mapView.lodIntensity = value
                        }

                        background: Item {}   // invisible — drawn above manually

                        handle: Rectangle {
                            x: lodSlider.visualPosition * (lodSlider.width - width)
                            y: (lodSlider.height - height) / 2
                            width: 16
                            height: 16
                            radius: 8
                            color: theme.accentColor
                            border.color: Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 0.4)
                            border.width: 3

                            // Glow ring
                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width + 8
                                height: parent.height + 8
                                radius: width / 2
                                color: "transparent"
                                border.color: Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, lodSlider.pressed ? 0.5 : 0.0)
                                border.width: 2
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }

                            Behavior on color { ColorAnimation { duration: 150 } }
                            scale: lodSlider.pressed ? 1.2 : 1.0
                            Behavior on scale { NumberAnimation { duration: 200; easing.type: Easing.OutBack } }
                        }
                    }
                }

                // Tick labels: Soft | Balanced | Aggressive
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Soft";       font.pixelSize: 10; color: theme.subTextColor; opacity: 0.6 }
                    Item { Layout.fillWidth: true }
                    Text { text: "Balanced";   font.pixelSize: 10; color: theme.subTextColor; opacity: 0.6 }
                    Item { Layout.fillWidth: true }
                    Text { text: "Aggressive"; font.pixelSize: 10; color: theme.subTextColor; opacity: 0.6 }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor }

            ColumnLayout {
                spacing: 12
                Layout.fillWidth: true

                Text {
                    text: "Node Glow Color"
                    color: theme.subTextColor
                    font.pixelSize: 12
                    Layout.fillWidth: true
                }
                Row {
                    spacing: 12
                    Repeater {
                        model: ["#00ffff", "#0284c7", "#f43f5e", "#10b981", "#eab308"]
                        delegate: Rectangle {
                            width: 24; height: 24; radius: 12
                            color: modelData
                            border.color: Qt.colorEqual(mapPalette.nodeGlow, modelData) ? theme.textColor : Qt.rgba(1, 1, 1, 0.2)
                            border.width: Qt.colorEqual(mapPalette.nodeGlow, modelData) ? 2 : 1
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: mapPalette.nodeGlow = modelData
                            }
                            Behavior on border.color { ColorAnimation { duration: 200 } }
                        }
                    }
                }
            }
            
            ColumnLayout {
                spacing: 12
                Layout.fillWidth: true

                Text {
                    text: "Edge Core Color"
                    color: theme.subTextColor
                    font.pixelSize: 12
                    Layout.fillWidth: true
                }
                Row {
                    spacing: 12
                    Repeater {
                        model: ["#00ffff", "#0284c7", "#22d3ee", "#10b981", "#3b82f6"]
                        delegate: Rectangle {
                            width: 24; height: 24; radius: 12
                            color: modelData
                            border.color: Qt.colorEqual(mapPalette.edgeCool, modelData) ? theme.textColor : Qt.rgba(1, 1, 1, 0.2)
                            border.width: Qt.colorEqual(mapPalette.edgeCool, modelData) ? 2 : 1
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: mapPalette.edgeCool = modelData
                            }
                            Behavior on border.color { ColorAnimation { duration: 200 } }
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
        }
        }


