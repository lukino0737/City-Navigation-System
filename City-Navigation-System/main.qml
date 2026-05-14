import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Navigation 1.0

Window {
    id: mainWindow
    width: 1280
    height: 800
    visible: true
    title: i18n.t("app.windowTitle")
    color: theme.bgColor
    flags: Qt.FramelessWindowHint | Qt.Window
    minimumWidth: 960
    minimumHeight: 640

    property bool globalIsGenerating: false
    property bool showingDetailPanel: false
    property string currentPage: "simulation"
    
    Connections {
        target: globalGraph
        function onGraphRegenerated() {
            globalIsGenerating = false
            if (mapView) mapView.refresh()
        }
    }

    Connections {
        target: mapView
        function onPathResultReady(info) {
            detailPanel.showInfo(info)
            showingDetailPanel = true
        }
        function onRouteModeChanged() {
            if (!mapView.routeMode) {
                showingDetailPanel = false
            }
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
            mapPalette.nodeCore = "#00ffff"
            mapPalette.nodeGlow = "#0f0f12"
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
            mapPalette.nodeCore = "#0284c7"
            mapPalette.nodeGlow = "#0f172a"
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
            mapPalette.nodeCore = "#22d3ee"
            mapPalette.nodeGlow = "#000000"
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
        property color virtualEdgeColor: "#808080"

        property color nodeCore: "#00ffff"
        property color nodeGlow: "#0f0f12"
        property real nodeCoreAlpha: 0.98
        property real nodeGlowAlpha: 0.5
        property real nodeGlowSizeScale: 2.0
    }

    // ── Localization Manager ──────────────────────────────────────────────
    QtObject {
        id: i18n

        property string language: "zh"

        property var _en: ({
            "app.title": "City Navigation System",
            "app.windowTitle": "City Navigation System - Premium Edition",
            "loading.title": "Generating Network...",
            "loading.subtitle": "10,000 Nodes | Prim's Algorithm",
            "tooltip.node": "NODE %1\n%2, %3",
            "tooltip.edge": "%1 ↔ %2\nCAPACITY: %3\nFLOW: %5\nLENGTH: %4",
            "sidebar.simulation": "Simulation",
            "sidebar.realmap": "Real Map",
            "sidebar.route": "Smart Routing",
            "sidebar.settings": "Settings",
            "rightpanel.header": "NAVIGATION",
            "rightpanel.title": "Control Center",
            "rightpanel.nodes": "Total Nodes",
            "rightpanel.zoom": "Zoom Level",
            "rightpanel.regen": "Regenerate Network",
            "rightpanel.generating": "Generating...",
            "rightpanel.lod": "LOD Intensity",
            "rightpanel.lod.full": "Full",
            "rightpanel.lod.soft": "Soft",
            "rightpanel.lod.bal": "Balanced",
            "rightpanel.lod.agg": "Aggressive",
            "rightpanel.autozoom.on": "Auto-Zoom LOD: ON",
            "rightpanel.autozoom.off": "Auto-Zoom LOD: OFF",
            "rightpanel.mapFile": "Current Map",
            "rightpanel.viewMode": "View Mode",
            "rightpanel.viewMode.traffic": "Traffic",
            "rightpanel.viewMode.centrality": "Centrality",
            "rightpanel.viewMode.original": "Original",
            "rightpanel.nodeColor": "Node Color",
            "rightpanel.edgeColor": "Edge Core Color",
            "settings.header": "PREFERENCES",
            "settings.title": "System Settings",
            "settings.theme": "Visual Theme",
            "settings.theme.dark": "Deep Space",
            "settings.theme.light": "Alabaster",
            "settings.theme.cyber": "Neon Obsidian",
            "settings.language": "Language",
            "settings.lang.zh": "中文",
            "settings.lang.en": "English",
            "settings.dismiss": "Dismiss",
            "detail.header.node": "NODE DETAILS",
            "detail.header.edge": "EDGE DETAILS",
            "detail.header.path": "ROUTE DETAILS",
            "detail.title.node": "Intersection #%1",
            "detail.title.edge": "Connection %1 ↔ %2",
            "detail.title.path.ok": "Route %1 → %2",
            "detail.title.path.fail": "No path found",
            "detail.node.id": "Node ID",
            "detail.node.name": "Name",
            "detail.node.x": "Position X",
            "detail.node.y": "Position Y",
            "detail.node.degree": "Degree",
            "detail.edge.source": "Source ID",
            "detail.edge.target": "Target ID",
            "detail.edge.capacity": "Capacity",
            "detail.edge.currentCars": "Current Flow",
            "detail.edge.length": "Length",
            "detail.path.start": "Start Node",
            "detail.path.startName": "Start Name",
            "detail.path.end": "End Node",
            "detail.path.endName": "End Name",
            "detail.path.cost": "Total Cost",
            "detail.path.hops": "Hops",
            "detail.path.nodes": "Nodes in Path",
            "detail.path.time": "Compute Time",
            "detail.path.result": "Result",
            "detail.path.timeUnit": " ms",
            "detail.path.noRoute": "No route found between these nodes",
            "detail.path.namedNodes": "Named Waypoints",
            "viewswitcher.controls": "Controls",
            "viewswitcher.details": "Details"
        })

        property var _zh: ({
            "app.title": "城市导航系统",
            "app.windowTitle": "城市导航系统",
            "loading.title": "正在加载地图...",
            "loading.subtitle": "10,000 个节点",
            "tooltip.node": "节点 %1\n%2, %3",
            "tooltip.edge": "%1 ↔ %2\n容量: %3\n当前车流: %5\n长度: %4",
            "sidebar.simulation": "模拟地图",
            "sidebar.realmap": "现实地图",
            "sidebar.route": "寻路模式",
            "sidebar.settings": "系统设定",
            "rightpanel.header": "导航",
            "rightpanel.title": "控制中心",
            "rightpanel.nodes": "节点总数",
            "rightpanel.zoom": "缩放级别",
            "rightpanel.regen": "重新生成网络",
            "rightpanel.generating": "生成中...",
            "rightpanel.lod": "LOD 强度",
            "rightpanel.lod.full": "完整",
            "rightpanel.lod.soft": "柔和",
            "rightpanel.lod.bal": "均衡",
            "rightpanel.lod.agg": "激进",
            "rightpanel.autozoom.on": "自动缩放 LOD: 开",
            "rightpanel.autozoom.off": "自动缩放 LOD: 关",
            "rightpanel.mapFile": "当前地图",
            "rightpanel.viewMode": "视图模式",
            "rightpanel.viewMode.traffic": "路况状态",
            "rightpanel.viewMode.centrality": "中心性视图",
            "rightpanel.viewMode.original": "原始视图",
            "rightpanel.nodeColor": "节点颜色",
            "rightpanel.edgeColor": "边核心颜色",
            "settings.header": "首选项",
            "settings.title": "系统设置",
            "settings.theme": "视觉主题",
            "settings.theme.dark": "深空",
            "settings.theme.light": "雪花石膏",
            "settings.theme.cyber": "霓虹黑曜",
            "settings.language": "语言",
            "settings.lang.zh": "中文",
            "settings.lang.en": "English",
            "settings.dismiss": "关闭",
            "detail.header.node": "节点详情",
            "detail.header.edge": "边详情",
            "detail.header.path": "路径详情",
            "detail.title.node": "节点 #%1",
            "detail.title.edge": "连接 %1 ↔ %2",
            "detail.title.path.ok": "路径 %1 → %2",
            "detail.title.path.fail": "未找到路径",
            "detail.node.id": "节点 ID",
            "detail.node.name": "地名",
            "detail.node.x": "横坐标",
            "detail.node.y": "纵坐标",
            "detail.node.degree": "度",
            "detail.edge.source": "起点 ID",
            "detail.edge.target": "终点 ID",
            "detail.edge.capacity": "容量",
            "detail.edge.currentCars": "当前车流",
            "detail.edge.length": "长度",
            "detail.path.start": "起点",
            "detail.path.startName": "起点地名",
            "detail.path.end": "终点",
            "detail.path.endName": "终点地名",
            "detail.path.cost": "总代价",
            "detail.path.hops": "跳数",
            "detail.path.nodes": "路径节点数",
            "detail.path.time": "计算时间",
            "detail.path.result": "结果",
            "detail.path.timeUnit": " 毫秒",
            "detail.path.noRoute": "未找到连接这两个节点的路径",
            "detail.path.namedNodes": "途经有名路口",
            "viewswitcher.controls": "控制面板",
            "viewswitcher.details": "详情"
        })

        // Pre-built model arrays so ComboBox model bindings avoid t() calls
        property var themeModel: language === "zh"
            ? [_zh["settings.theme.dark"], _zh["settings.theme.light"], _zh["settings.theme.cyber"]]
            : [_en["settings.theme.dark"], _en["settings.theme.light"], _en["settings.theme.cyber"]]

        property var langModel: language === "zh"
            ? [_zh["settings.lang.zh"], _zh["settings.lang.en"]]
            : [_en["settings.lang.zh"], _en["settings.lang.en"]]

        function t(key) {
            var map = language === "zh" ? _zh : _en
            var text = map[key]
            if (text === undefined) return key
            for (var i = 1; i < arguments.length; i++) {
                text = text.replace("%" + i, arguments[i])
            }
            return text
        }
    }

    // ── Custom Acrylic Title Bar ───────────────────────────────────────
    TitleBar {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        z: 20
        targetWindow: mainWindow
        blurSource: mapContainer
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
            virtualEdgeColor: mapPalette.virtualEdgeColor
            nodeCoreColor: mapPalette.nodeCore
            nodeGlowColor: mapPalette.nodeGlow
            nodeCoreAlpha: mapPalette.nodeCoreAlpha
            nodeGlowAlpha: mapPalette.nodeGlowAlpha
            nodeGlowSizeScale: mapPalette.nodeGlowSizeScale

            onSelectionInfoReady: function(info) {
                detailPanel.showInfo(info)
            }
            
            selectionAccent: theme.accentColor

            // Map Interactions
            MouseArea {
                id: mapMouse
                anchors.fill: parent
                scrollGestureEnabled: true
                hoverEnabled: true

                property point lastPos: Qt.point(0, 0)
                property point pressPos: Qt.point(0, 0)
                property bool isDragging: false
                readonly property real dragThreshold: 5

                function updateHoverState(mx, my) {
                    let hoveredNodePos = mapView.hitTestNode(Qt.point(mx, my), 10.0)
                    if (hoveredNodePos.found) {
                        nodeTooltip.x = mx + 20
                        nodeTooltip.y = my + 20
                        var nodeText = i18n.t("tooltip.node", hoveredNodePos.id, hoveredNodePos.x, hoveredNodePos.y)
                        if (hoveredNodePos.name && hoveredNodePos.name !== "") {
                            nodeText = hoveredNodePos.name + "\n" + nodeText
                        }
                        tooltipText.text = nodeText
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
                        tooltipText.text = i18n.t("tooltip.edge", hoveredEdge.source, hoveredEdge.target, hoveredEdge.capacity, hoveredEdge.length.toFixed(1), hoveredEdge.currentCars !== undefined ? hoveredEdge.currentCars : "—")
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
                    pressPos = Qt.point(mouse.x, mouse.y)
                    lastPos = Qt.point(mouse.x, mouse.y)
                    isDragging = false
                    updateHoverState(-1000, -1000)
                }

                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let dist = Math.hypot(mouse.x - pressPos.x, mouse.y - pressPos.y)
                        if (dist > dragThreshold) isDragging = true
                        if (isDragging) {
                            let dx = (mouse.x - lastPos.x) / mapView.width
                            let dy = (mouse.y - lastPos.y) / mapView.height
                            mapView.offset = Qt.point(mapView.offset.x + dx, mapView.offset.y + dy)
                            lastPos = Qt.point(mouse.x, mouse.y)
                        }
                    } else {
                        updateHoverState(mouse.x, mouse.y)
                    }
                }

                onReleased: (mouse) => {
                    if (!isDragging) handleMapClick(mouse.x, mouse.y)
                }

                onWheel: (wheel) => {
                    mapView.zoomAtPoint(wheel.angleDelta.y / 120.0, wheel.x, wheel.y)
                }

                function handleMapClick(mx, my) {
                    // Route mode: two-click node selection for pathfinding
                    if (mapView.routeMode) {
                        let nodeHit = mapView.hitTestNode(Qt.point(mx, my), 12.0)
                        if (nodeHit.found) {
                            mapView.selectRouteNode(nodeHit.id)
                            showingDetailPanel = true
                            return
                        }
                        mapView.clearRoute()
                        showingDetailPanel = false
                        return
                    }

                    // View mode: single-selection for node/edge details
                    let nodeHit = mapView.hitTestNode(Qt.point(mx, my), 12.0)
                    if (nodeHit.found) {
                        let info = mapView.selectNode(nodeHit.id)
                        detailPanel.showInfo(info)
                        showingDetailPanel = true
                        return
                    }
                    let edgeHit = mapView.hitTestEdge(Qt.point(mx, my), 6.0)
                    if (edgeHit.found) {
                        let info = mapView.selectEdge(edgeHit.source, edgeHit.target)
                        detailPanel.showInfo(info)
                        showingDetailPanel = true
                        return
                    }
                    mapView.clearSelection()
                    showingDetailPanel = false
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
                        text: i18n.t("loading.title")
                        color: theme.textColor
                        font.pixelSize: 18
                        font.weight: Font.Medium
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: i18n.t("loading.subtitle")
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
        anchors.topMargin: 64
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        z: 10
        mapView: mapView
        blurTarget: mapContainer
        currentPage: mainWindow.currentPage
        onSimulationClicked: {
            mainWindow.currentPage = "simulation"
            mapView.clearRoute()
            showingDetailPanel = false
            globalIsGenerating = true
            globalGraph.reloadSimulationMap()
        }
        onRealMapClicked: {
            mainWindow.currentPage = "real"
            mapView.clearRoute()
            showingDetailPanel = false
            if (globalGraph.availableMaps.length > 0) {
                globalIsGenerating = true
                globalGraph.switchToMap(globalGraph.currentMapIndex >= 0 ? globalGraph.currentMapIndex : 0)
            }
        }
        onSettingsClicked: settingsDialog.visible = true
    }

    // Floating Right Dashboard
    RightPanel {
        id: rightPanel
        y: 64
        height: parent.height - 88
        z: 10
        mapView: mapView
        blurTarget: mapContainer
        isGenerating: globalIsGenerating
        active: !showingDetailPanel
        currentPage: mainWindow.currentPage
        onRegenerateClicked: {
            mapView.clearRoute()
            globalIsGenerating = true
            globalGraph.regenerateGraph(10000)
        }
        onMapSwitchRequested: {
            globalIsGenerating = true
        }
        onRouteClearRequested: {
            showingDetailPanel = false
        }
    }
    
    // Settings Overlay
    SettingsDialog {
        id: settingsDialog
        blurTarget: mapContainer
    }

    DetailPanel {
        id: detailPanel
        y: 64
        width: 340
        height: parent.height - 88
        z: 11
        blurTarget: mapContainer
        
        x: (mapView.selectionMode !== "none" && showingDetailPanel) 
           ? parent.width - width - 24 
           : parent.width + 24
           
        Behavior on x { NumberAnimation { duration: 450; easing.type: Easing.OutQuint } }
    }

    ViewSwitcher {
        id: switcher
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 48
        anchors.right: parent.right
        anchors.rightMargin: 24 + (rightPanel.width - width) / 2
        z: 15
        
        detailsEnabled: mapView.selectionMode !== "none"
        currentIndex: showingDetailPanel ? 1 : 0
        
        visible: detailsEnabled
        opacity: visible ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: 300 } }
        
        onSwitched: (index) => {
            showingDetailPanel = (index === 1)
        }
    }

    // Bottom blur gradient mask below ViewSwitcher
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80
        z: 14
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.5; color: Qt.rgba(theme.bgColor.r, theme.bgColor.g, theme.bgColor.b, 0.7) }
            GradientStop { position: 1.0; color: theme.bgColor }
        }
    }
}
