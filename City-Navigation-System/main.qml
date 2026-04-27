import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Navigation 1.0

Window {
    id: mainWindow
    width: 1280
    height: 800
    visible: true
    title: qsTr("City Navigation System - Premium Edition")
    color: theme.bgColor 
    
    property bool globalIsGenerating: false
    
    Connections {
        target: globalGraph
        function onGraphRegenerated() {
            globalIsGenerating = false
            if (mapView) mapView.update()
        }
    }

    // Premium Color Palette Manager
    QtObject {
        id: theme
        
        property color bgColor: "#111116"
        property color textColor: "#f8f9fa"
        property color subTextColor: "#94a3b8"
        property color accentColor: "#3b82f6" 
        property color secondaryAccent: "#6366f1"
        property color panelColor: Qt.rgba(0.12, 0.12, 0.16, 0.45)
        property color panelBorder: Qt.rgba(1, 1, 1, 0.15)
        property color activePill: Qt.rgba(0.23, 0.51, 0.96, 0.3)
        property color dividerColor: Qt.rgba(1, 1, 1, 0.1)
        property color buttonBg: Qt.rgba(1, 1, 1, 0.04)
        property color buttonHover: Qt.rgba(1, 1, 1, 0.08)
        property color buttonDown: Qt.rgba(1, 1, 1, 0.02)
        
        function setDarkTheme() {
            bgColor = "#111116"
            textColor = "#f8f9fa"
            subTextColor = "#94a3b8"
            accentColor = "#3b82f6"
            secondaryAccent = "#6366f1"
            panelColor = Qt.rgba(0.12, 0.12, 0.16, 0.45)
            panelBorder = Qt.rgba(1, 1, 1, 0.15)
            activePill = Qt.rgba(0.23, 0.51, 0.96, 0.3)
            dividerColor = Qt.rgba(1, 1, 1, 0.1)
            buttonBg = Qt.rgba(1, 1, 1, 0.04)
            buttonHover = Qt.rgba(1, 1, 1, 0.12)
            buttonDown = Qt.rgba(1, 1, 1, 0.02)
            
            mapPalette.edgeCool = "#00ffff"
            mapPalette.edgeMid = "#7f00ff"
            mapPalette.edgeWarm = "#ff00ff"
            mapPalette.nodeCore = "#0f0f12"
            mapPalette.nodeGlow = "#00ffff"
            mapPalette.edgeGlowAlpha = 0.28
        }
        
        function setLightTheme() {
            bgColor = "#f0f2f5"
            textColor = "#0f172a"
            subTextColor = "#64748b"
            accentColor = "#0284c7"
            secondaryAccent = "#0ea5e9"
            panelColor = Qt.rgba(1, 1, 1, 0.45)
            panelBorder = Qt.rgba(1, 1, 1, 0.9)
            activePill = Qt.rgba(0.0, 0.4, 0.8, 0.15)
            dividerColor = Qt.rgba(0, 0, 0, 0.08)
            buttonBg = Qt.rgba(0, 0, 0, 0.03)
            buttonHover = Qt.rgba(0, 0, 0, 0.08)
            buttonDown = Qt.rgba(0, 0, 0, 0.02)

            mapPalette.edgeCool = "#0284c7"
            mapPalette.edgeMid = "#8b5cf6"
            mapPalette.edgeWarm = "#e11d48"
            mapPalette.nodeCore = "#0f172a"
            mapPalette.nodeGlow = "#0284c7"
            mapPalette.edgeGlowAlpha = 0.15
        }
        
        function setCyberTheme() {
            bgColor = "#0f0914"
            textColor = "#e0f2fe"
            subTextColor = "#f472b6"
            accentColor = "#22d3ee"
            secondaryAccent = "#d946ef"
            panelColor = Qt.rgba(0.08, 0.05, 0.15, 0.7)
            panelBorder = Qt.rgba(0.13, 0.83, 0.93, 0.3)
            activePill = Qt.rgba(0.13, 0.83, 0.93, 0.2)
            dividerColor = Qt.rgba(0.13, 0.83, 0.93, 0.15)
            buttonBg = Qt.rgba(0.13, 0.83, 0.93, 0.05)
            buttonHover = Qt.rgba(0.13, 0.83, 0.93, 0.15)
            buttonDown = Qt.rgba(0.13, 0.83, 0.93, 0.02)
            
            mapPalette.edgeCool = "#22d3ee"
            mapPalette.edgeMid = "#d946ef"
            mapPalette.edgeWarm = "#f43f5e"
            mapPalette.nodeCore = "#000000"
            mapPalette.nodeGlow = "#22d3ee"
            mapPalette.edgeGlowAlpha = 0.4
        }
    }

    QtObject {
        id: mapPalette

        property color edgeCool: "#00ffff"
        property color edgeMid: "#7f00ff"
        property color edgeWarm: "#ff00ff"
        property color edgeHover: "#ffffff"

        property real edgeMidRatio: 0.55
        property real edgeCoreAlpha: 0.95
        property real edgeGlowAlpha: 0.28
        property real edgeGlowWidthScale: 1.9

        property color nodeCore: "#0f0f12"
        property color nodeGlow: "#00ffff"
        property real nodeCoreAlpha: 0.98
        property real nodeGlowAlpha: 0.5
        property real nodeGlowSizeScale: 2.0
    }

        // Core Map Rendering Container
    Item {
        id: mapContainer
        anchors.fill: parent
        
        Rectangle {
            anchors.fill: parent
            color: theme.bgColor
        }

        MapView {
            id: mapView
            anchors.fill: parent
            graph: globalGraph
            offset: Qt.point(0.05, 0.05)
            edgeCoolColor: mapPalette.edgeCool
            edgeMidColor: mapPalette.edgeMid
            edgeWarmColor: mapPalette.edgeWarm
            edgeHoverColor: mapPalette.edgeHover
            edgeMidRatio: mapPalette.edgeMidRatio
            edgeCoreAlpha: mapPalette.edgeCoreAlpha
            edgeGlowAlpha: mapPalette.edgeGlowAlpha
            edgeGlowWidthScale: mapPalette.edgeGlowWidthScale
            nodeCoreColor: mapPalette.nodeCore
            nodeGlowColor: mapPalette.nodeGlow
            nodeCoreAlpha: mapPalette.nodeCoreAlpha
            nodeGlowAlpha: mapPalette.nodeGlowAlpha
            nodeGlowSizeScale: mapPalette.nodeGlowSizeScale

            // Map Interactions
            MouseArea {
                anchors.fill: parent
                scrollGestureEnabled: true
                hoverEnabled: true
                
                property point lastPos: Qt.point(0, 0)
                
                function updateHoverState(mx, my) {
                    let hoveredNodePos = mapView.hitTestNode(Qt.point(mx, my), 10.0)
                    if (hoveredNodePos.found) {
                        nodeTooltip.x = mx + 20
                        nodeTooltip.y = my + 20
                        tooltipText.text = "NODE " + hoveredNodePos.id + "\n" + hoveredNodePos.x + ", " + hoveredNodePos.y
                        nodeTooltip.visible = true
                        
                        let screenPos = mapView.mapToScreen(hoveredNodePos.x, hoveredNodePos.y)
                        hoverNodeIndicator.x = screenPos.x - hoverNodeIndicator.width / 2
                        hoverNodeIndicator.y = screenPos.y - hoverNodeIndicator.height / 2
                        hoverNodeIndicator.visible = true
                        hoverNodeIndicator.scale = 1.0
                        
                        mapView.hoveredEdgeSource = -1
                        mapView.hoveredEdgeTarget = -1
                        return
                    }
                    
                    let hoveredEdge = mapView.hitTestEdge(Qt.point(mx, my), 5.0)
                    if (hoveredEdge.found) {
                        nodeTooltip.x = mx + 20
                        nodeTooltip.y = my + 20
                        tooltipText.text = hoveredEdge.source + " ↔ " + hoveredEdge.target + "\nCAPACITY: " + hoveredEdge.capacity + "\nLENGTH: " + hoveredEdge.length.toFixed(1)
                        nodeTooltip.visible = true
                        
                        hoverNodeIndicator.visible = false
                        
                        mapView.hoveredEdgeSource = hoveredEdge.source
                        mapView.hoveredEdgeTarget = hoveredEdge.target
                        return
                    }
                    
                    nodeTooltip.visible = false
                    hoverNodeIndicator.visible = false
                    
                    mapView.hoveredEdgeSource = -1
                    mapView.hoveredEdgeTarget = -1
                }

                onPressed: (mouse) => {
                    lastPos = Qt.point(mouse.x, mouse.y)
                    updateHoverState(-1000, -1000)
                }

                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let dx = (mouse.x - lastPos.x) / mapView.width
                        let dy = (mouse.y - lastPos.y) / mapView.height
                        mapView.offset = Qt.point(mapView.offset.x + dx, mapView.offset.y + dy)
                        lastPos = Qt.point(mouse.x, mouse.y)
                    } else {
                        updateHoverState(mouse.x, mouse.y)
                    }
                }

                onWheel: (wheel) => {
                    mapView.addZoomVelocity(wheel.angleDelta.y / 120.0)
                }
            }
        }

        MultiEffect {
            source: mapView
            anchors.fill: mapView
            blurEnabled: true
            blurMax: 64
            blur: mapView.momentumActive ? 0.25 : 1.0
            opacity: 0.6

            Behavior on blur {
                NumberAnimation { duration: 250; easing.type: Easing.OutQuad }
            }
        }

        // Invisible blocker to prevent map interaction during load
        MouseArea {
            anchors.fill: parent
            visible: globalIsGenerating
            hoverEnabled: true
            preventStealing: true
            z: 89
        }

        // Premium Loading Card
        GlassPanel {
            id: loadingOverlay
            width: 380
            height: 240
            anchors.centerIn: parent
            radius: 24
            visible: globalIsGenerating
            z: 90
            blurSource: mapView

            Column {
                anchors.centerIn: parent
                spacing: 24

                Item {
                    width: 64
                    height: 64
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        anchors.fill: parent
                        radius: 32
                        color: "transparent"
                        border.color: Qt.rgba(1, 1, 1, 0.1)
                        border.width: 4
                    }

                    Rectangle {
                        width: parent.width
                        height: parent.height
                        radius: width / 2
                        color: "transparent"
                        border.color: theme.accentColor
                        border.width: 4

                        gradient: Gradient {
                            GradientStop { position: 0.0; color: theme.accentColor }
                            GradientStop { position: 0.5; color: "transparent" }
                        }

                        RotationAnimation on rotation {
                            from: 0
                            to: 360
                            duration: 1000
                            loops: Animation.Infinite
                            running: loadingOverlay.visible
                        }
                    }
                }

                Column {
                    spacing: 8
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        text: "Generating Network..."
                        color: theme.textColor
                        font.pixelSize: 18
                        font.weight: Font.Medium
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: "10,000 Nodes | Prim's Algorithm"
                        color: theme.subTextColor
                        font.pixelSize: 12
                        font.letterSpacing: 2
                        font.weight: Font.Light
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutExpo } }
            opacity: visible ? 1.0 : 0.0
        }
    }

    // Elegant Tooltip
    GlassPanel {
        id: nodeTooltip
        visible: false
        width: tooltipText.width + 32
        height: tooltipText.height + 20
        radius: 12
        z: 100
        
        blurSource: mapContainer

        Text {
            id: tooltipText
            anchors.centerIn: parent
            color: theme.textColor
            font.pixelSize: 12
            font.weight: Font.Medium
            text: ""
        }

        Behavior on x { NumberAnimation { duration: 250; easing.type: Easing.OutExpo } }
        Behavior on y { NumberAnimation { duration: 250; easing.type: Easing.OutExpo } }
        Behavior on opacity { NumberAnimation { duration: 200 } }
    }

    // Subtly Glowing Hover Indicator
    Rectangle {
        id: hoverNodeIndicator
        width: 14
        height: 14
        radius: 7
        color: theme.accentColor
        visible: false
        z: 99
        
        Rectangle {
            anchors.fill: parent
            anchors.margins: -6
            radius: width / 2
            color: theme.accentColor
            opacity: 0.3
        }
        
        Behavior on scale {
            NumberAnimation { duration: 400; easing.type: Easing.OutElastic }
        }
    }

    // Floating Left Dock
    Sidebar {
        id: sidebar
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        z: 10
        mapView: mapView
        blurTarget: mapContainer
        onSettingsClicked: settingsDialog.visible = true
    }

    // Floating Right Dashboard
    RightPanel {
        id: rightPanel
        y: 24
        height: parent.height - 48
        z: 10
        mapView: mapView
        blurTarget: mapContainer
        isGenerating: globalIsGenerating
        onRegenerateClicked: {
            globalIsGenerating = true
            globalGraph.regenerateGraph(10000)
        }
    }
    
    // Settings Overlay
    SettingsDialog {
        id: settingsDialog
        blurTarget: mapContainer
    }
}
