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

        // 处理鼠标交互：缩放与平移
        MouseArea {
            anchors.fill: parent
            scrollGestureEnabled: true
            
            property point lastPos: Qt.point(0, 0)

            onPressed: (mouse) => {
                lastPos = Qt.point(mouse.x, mouse.y)
            }

            onPositionChanged: (mouse) => {
                if (pressed) {
                    var dx = (mouse.x - lastPos.x) / mapView.width
                    var dy = (mouse.y - lastPos.y) / mapView.height
                    mapView.offset = Qt.point(mapView.offset.x + dx, mapView.offset.y + dy)
                    lastPos = Qt.point(mouse.x, mouse.y)
                }
            }

            onWheel: (wheel) => {
                var delta = wheel.angleDelta.y / 1200.0
                mapView.zoom = Math.max(0.1, Math.min(50.0, mapView.zoom + delta))
            }
        }
    }

    // 状态栏
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 30
        color: "#333333"
        opacity: 0.8

        Text {
            anchors.centerIn: parent
            color: "white"
            text: "节点数: 10000 | 缩放: " + mapView.zoom.toFixed(2) + " | 拖拽地图以移动，滚轮缩放"
        }
    }
}
