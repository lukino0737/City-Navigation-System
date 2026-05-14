import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root
    width: 340

    property bool isCollapsed: false
    property var mapView: null
    property Item blurTarget: null
    property bool isGenerating: false
    property bool active: true
    property string currentPage: "simulation"
    signal regenerateClicked()
    signal mapSwitchRequested()
    signal routeClearRequested()

    Behavior on x { NumberAnimation { duration: 450; easing.type: Easing.OutQuint } }

    // Sliding logic + Offset when inactive
    x: {
        let baseX = isCollapsed ? parent.width : parent.width - width - 24
        return active ? baseX : baseX - 40
    }
    
    opacity: active ? 1.0 : 0.0
    scale: active ? 1.0 : 0.95
    
    Behavior on opacity { NumberAnimation { duration: 400; easing.type: Easing.OutCubic } }
    Behavior on scale { NumberAnimation { duration: 400; easing.type: Easing.OutBack } }

    // 阻止鼠标事件穿透到下方的地图
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
        enabled: root.active
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
        visible: root.active
        
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

    Flickable {
        id: panelFlickable
        anchors.fill: parent
        contentWidth: width
        contentHeight: innerColumn.implicitHeight + 80
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.top: parent.top
            anchors.topMargin: 32
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 32
        }

        ColumnLayout {
            id: innerColumn
            width: parent.width - 64
            x: 32
            y: 32
            spacing: 28

        ColumnLayout {
            spacing: 4
            Text {
                text: i18n.t("rightpanel.header")
                color: theme.accentColor
                font.pixelSize: 11
                font.letterSpacing: 2
                font.weight: Font.Bold
            }
            Text {
                text: i18n.t("rightpanel.title")
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

            Text { text: i18n.t("rightpanel.nodes"); color: theme.subTextColor; font.pixelSize: 13 }
            Text { text: "10,000"; color: theme.textColor; font.pixelSize: 14; font.weight: Font.Medium; Layout.alignment: Qt.AlignRight }

            Text { text: i18n.t("rightpanel.zoom"); color: theme.subTextColor; font.pixelSize: 13 }
            Text { text: (mapView ? mapView.zoom.toFixed(2) : "1.0") + "x"; color: theme.textColor; font.pixelSize: 14; font.weight: Font.Medium; Layout.alignment: Qt.AlignRight }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor; visible: root.currentPage === "real" }

        // Map file selector (only in real map mode)
        RowLayout {
            visible: root.currentPage === "real"
            Layout.fillWidth: true
            spacing: 24
            Text {
                text: i18n.t("rightpanel.mapFile")
                color: theme.subTextColor
                font.pixelSize: 14
                Layout.fillWidth: true
            }

            ComboBox {
                id: mapCombo
                model: globalGraph.availableMaps
                currentIndex: globalGraph.currentMapIndex
                Layout.preferredWidth: 180
                font.pixelSize: 13
                enabled: model && model.length > 0
                onCurrentIndexChanged: {
                    if (root.mapView) root.mapView.clearRoute()
                    root.routeClearRequested()
                    globalGraph.switchToMap(currentIndex)
                    root.mapSwitchRequested()
                }

                delegate: ItemDelegate {
                    width: mapCombo.width - 8
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
                    highlighted: mapCombo.highlightedIndex === index
                }

                indicator: Canvas {
                    x: mapCombo.width - width - 12
                    y: (mapCombo.height - height) / 2
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
                    border.color: mapCombo.visualFocus ? theme.accentColor : theme.panelBorder
                    border.width: 1
                    radius: 10
                    Behavior on border.color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: mapCombo.displayText
                    font: mapCombo.font
                    color: theme.textColor
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    leftPadding: 16
                    rightPadding: 32
                }

                popup: Popup {
                    y: mapCombo.height + 6
                    width: mapCombo.width
                    implicitHeight: contentItem.implicitHeight + 16
                    padding: 8

                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: mapCombo.popup.visible ? mapCombo.delegateModel : null
                        currentIndex: mapCombo.highlightedIndex
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

        // View Mode Selector
        RowLayout {
            Layout.fillWidth: true
            spacing: 24
            Text {
                text: i18n.t("rightpanel.viewMode")
                color: theme.subTextColor
                font.pixelSize: 14
                Layout.fillWidth: true
            }

            ComboBox {
                id: viewModeCombo
                model: [i18n.t("rightpanel.viewMode.traffic"), i18n.t("rightpanel.viewMode.centrality"), i18n.t("rightpanel.viewMode.original")]
                currentIndex: mapView ? mapView.edgeViewMode : 0
                Layout.preferredWidth: 180
                font.pixelSize: 13
                onCurrentIndexChanged: {
                    if (mapView) mapView.edgeViewMode = currentIndex
                }

                delegate: ItemDelegate {
                    width: viewModeCombo.width - 8
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
                    highlighted: viewModeCombo.highlightedIndex === index
                }

                indicator: Canvas {
                    x: viewModeCombo.width - width - 12
                    y: (viewModeCombo.height - height) / 2
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
                    border.color: viewModeCombo.visualFocus ? theme.accentColor : theme.panelBorder
                    border.width: 1
                    radius: 10
                    Behavior on border.color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: viewModeCombo.displayText
                    font: viewModeCombo.font
                    color: theme.textColor
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    leftPadding: 16
                    rightPadding: 32
                }

                popup: Popup {
                    y: viewModeCombo.height + 6
                    width: viewModeCombo.width
                    implicitHeight: contentItem.implicitHeight + 16
                    padding: 8

                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: viewModeCombo.popup.visible ? viewModeCombo.delegateModel : null
                        currentIndex: viewModeCombo.highlightedIndex
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

        Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor; visible: root.currentPage === "real" }

        ColumnLayout {
            spacing: 12
            Layout.fillWidth: true

            Button {
                id: regenBtn
                Layout.fillWidth: true
                text: root.isGenerating ? i18n.t("rightpanel.generating") : i18n.t("rightpanel.regen")
                enabled: !root.isGenerating
                onClicked: {
                    if (!root.isGenerating) {
                        root.regenerateClicked()
                    }
                }
                contentItem: Text {
                    text: root.isGenerating ? i18n.t("rightpanel.generating") : i18n.t("rightpanel.regen")
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

            // LOD Intensity Slider
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: i18n.t("rightpanel.lod")
                        color: theme.subTextColor
                        font.pixelSize: 12
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: {
                            if (!mapView) return "–"
                            let v = mapView.lodIntensity
                            if (v <= 0.05) return i18n.t("rightpanel.lod.full") + " (" + v.toFixed(1) + ")"
                            if (v < 0.9)  return i18n.t("rightpanel.lod.soft") + " (" + v.toFixed(1) + ")"
                            if (v < 2.1)  return i18n.t("rightpanel.lod.bal") + " (" + v.toFixed(1) + ")"
                            return i18n.t("rightpanel.lod.agg") + " (" + v.toFixed(1) + ")"
                        }
                        color: theme.accentColor
                        font.pixelSize: 11
                        font.weight: Font.Medium
                    }
                }

                Item {
                    Layout.fillWidth: true
                    height: 24

                    Rectangle {
                        width: parent.width
                        height: 4
                        anchors.verticalCenter: parent.verticalCenter
                        radius: 2
                        color: theme.dividerColor
                    }

                    Rectangle {
                        width: lodSlider.visualPosition * parent.width
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
                        from: 0.0
                        to: 3.0
                        stepSize: 0.1
                        value: mapView ? mapView.lodIntensity : 1.0

                        onMoved: {
                            if (mapView) mapView.lodIntensity = value
                        }

                        background: Item {}
                        handle: Rectangle {
                            x: lodSlider.visualPosition * (lodSlider.width - width)
                            y: (lodSlider.height - height) / 2
                            width: 16; height: 16; radius: 8
                            color: theme.accentColor
                            border.color: Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 0.4)
                            border.width: 3
                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width + 8; height: parent.height + 8; radius: width / 2
                                color: "transparent"
                                border.color: Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, lodSlider.pressed ? 0.5 : 0.0)
                                border.width: 2
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }
                            scale: lodSlider.pressed ? 1.2 : 1.0
                            Behavior on scale { NumberAnimation { duration: 200; easing.type: Easing.OutBack } }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: i18n.t("rightpanel.lod.full");      font.pixelSize: 10; color: theme.subTextColor; opacity: 0.6 }
                    Item { Layout.fillWidth: true }
                    Text { text: i18n.t("rightpanel.lod.soft");      font.pixelSize: 10; color: theme.subTextColor; opacity: 0.6 }
                    Item { Layout.fillWidth: true }
                    Text { text: i18n.t("rightpanel.lod.bal");  font.pixelSize: 10; color: theme.subTextColor; opacity: 0.6 }
                    Item { Layout.fillWidth: true }
                    Text { text: i18n.t("rightpanel.lod.agg");font.pixelSize: 10; color: theme.subTextColor; opacity: 0.6 }
                }

                // Auto-zoom toggle
                Button {
                    id: autoZoomBtn
                    Layout.fillWidth: true
                    text: mapView && mapView.lodAutoZoom ? i18n.t("rightpanel.autozoom.on") : i18n.t("rightpanel.autozoom.off")
                    onClicked: {
                        if (mapView) mapView.lodAutoZoom = !mapView.lodAutoZoom
                    }
                    contentItem: Text {
                        text: mapView && mapView.lodAutoZoom ? i18n.t("rightpanel.autozoom.on") : i18n.t("rightpanel.autozoom.off")
                        color: (mapView && mapView.lodAutoZoom) ? theme.accentColor : theme.subTextColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 12
                        font.weight: Font.Medium
                    }
                    background: Rectangle {
                        implicitHeight: 38
                        radius: 10
                        color: (mapView && mapView.lodAutoZoom) ? theme.activePill : (autoZoomBtn.hovered ? theme.buttonHover : "transparent")
                        border.color: (mapView && mapView.lodAutoZoom) ? Qt.rgba(theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 0.4) : (autoZoomBtn.hovered ? theme.panelBorder : "transparent")
                        border.width: 1
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor }

            ColumnLayout {
                spacing: 12
                Layout.fillWidth: true

                Text {
                    text: i18n.t("rightpanel.nodeColor")
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
                            border.color: Qt.colorEqual(mapPalette.nodeCore, modelData) ? theme.textColor : Qt.rgba(1, 1, 1, 0.2)
                            border.width: Qt.colorEqual(mapPalette.nodeCore, modelData) ? 2 : 1
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: mapPalette.nodeCore = modelData
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
                    text: i18n.t("rightpanel.edgeColor")
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
        Item { height: 48 }
        }
    }

    // Gradient mask at bottom of panel — fade indicator for scrollable content
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: theme.panelColor }
        }
    }
}
