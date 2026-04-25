import QtQuick
import QtQuick.Controls
import Navigation 1.0

Window {
    width: 1024
    height: 768
    visible: true
    title: qsTr("城市导航模拟系统 - 10,000 节点渲染")
    color: "#1e1e1e" // 暗色背景

    // 核心地图渲染组件
    MapView {
        id: mapView
        anchors.fill: parent
        graph: globalGraph // 绑定到 C++ 注入的全局变量
        zoom: 0.9 // 初始缩放
        offset: Qt.point(0.05, 0.05) // 初始偏移，留出边距

        // 工具提示框（Node）
        Rectangle {
            id: nodeTooltip
            visible: false
            width: tooltipText.width + 20
            height: tooltipText.height + 14
            color: "#CC1e1e1e" // 半透明暗色背景
            radius: 8
            border.color: "#4FC3F7"
            border.width: 1
            z: 100

            // 阴影效果
            layer.enabled: true

            Text {
                id: tooltipText
                anchors.centerIn: parent
                color: "white"
                font.pixelSize: 12
                text: ""
            }

            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutExpo } }
            Behavior on y { NumberAnimation { duration: 150; easing.type: Easing.OutExpo } }
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        // 高亮选中节点（缩放动画）
        Rectangle {
            id: hoverNodeIndicator
            width: 10
            height: 10
            radius: 5
            color: "#FF4081"
            visible: false
            z: 99
            
            Behavior on scale {
                NumberAnimation { duration: 250; easing.type: Easing.OutBack }
            }
        }

        // 处理鼠标交互：缩放与平移
        MouseArea {
            anchors.fill: parent
            scrollGestureEnabled: true
            hoverEnabled: true
            
            property point lastPos: Qt.point(0, 0)
            
            function updateHoverState(mx, my) {
                let hoveredNodePos = mapView.hitTestNode(Qt.point(mx, my), 10.0)
                if (hoveredNodePos.found) {
                    nodeTooltip.x = mx + 15
                    nodeTooltip.y = my + 15
                    tooltipText.text = "节点 ID: " + hoveredNodePos.id + "\n坐标: (" + hoveredNodePos.x + ", " + hoveredNodePos.y + ")"
                    nodeTooltip.visible = true
                    
                    let screenPos = mapView.mapToScreen(hoveredNodePos.x, hoveredNodePos.y)
                    hoverNodeIndicator.x = screenPos.x - hoverNodeIndicator.width / 2
                    hoverNodeIndicator.y = screenPos.y - hoverNodeIndicator.height / 2
                    hoverNodeIndicator.visible = true
                    hoverNodeIndicator.scale = 1.3
                    
                    mapView.hoveredEdgeSource = -1
                    mapView.hoveredEdgeTarget = -1
                    return
                }
                
                let hoveredEdge = mapView.hitTestEdge(Qt.point(mx, my), 5.0)
                if (hoveredEdge.found) {
                    nodeTooltip.x = mx + 15
                    nodeTooltip.y = my + 15
                    tooltipText.text = "路线连结: " + hoveredEdge.source + " ↔ " + hoveredEdge.target + "\n容量: " + hoveredEdge.capacity + "\n长度: " + hoveredEdge.length.toFixed(2)
                    nodeTooltip.visible = true
                    
                    hoverNodeIndicator.visible = false
                    hoverNodeIndicator.scale = 1.0
                    
                    mapView.hoveredEdgeSource = hoveredEdge.source
                    mapView.hoveredEdgeTarget = hoveredEdge.target
                    return
                }
                
                nodeTooltip.visible = false
                hoverNodeIndicator.visible = false
                hoverNodeIndicator.scale = 1.0
                
                mapView.hoveredEdgeSource = -1
                mapView.hoveredEdgeTarget = -1
            }

            onPressed: (mouse) => {
                lastPos = Qt.point(mouse.x, mouse.y)
                updateHoverState(-1000, -1000) // 隐藏提示
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
                let delta = wheel.angleDelta.y / 1200.0
                mapView.zoom = Math.max(0.1, Math.min(50.0, mapView.zoom + delta))
                if (!pressed) {
                    updateHoverState(wheel.x, wheel.y)
                }
            }
        }
    }

    // 状态栏
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 60
        color: "#333333"
        opacity: 0.9

        Row {
            anchors.centerIn: parent
            spacing: 20

            Text {
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                font.pixelSize: 14
                text: "节点数: 10000 | 缩放: " + mapView.zoom.toFixed(2) + " | 拖拽地图以移动，滚轮缩放"
            }

            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "重新生成图并刷新"
                onClicked: {
                    globalGraph.regenerateGraph(10000)
                    mapView.update()
                }
            }

            // LOD 开关按钮
            Button {
                anchors.verticalCenter: parent.verticalCenter
                // 按钮文字随状态动态更新
                text: mapView.lodEnabled ? "LOD: 开启 ✓" : "LOD: 关闭"
                // 高亮样式区分开启/关闭状态
                palette.button: mapView.lodEnabled ? "#2e7d32" : "#555555"
                palette.buttonText: "white"
                onClicked: {
                    mapView.lodEnabled = !mapView.lodEnabled
                }
            }
        }
    }
}
