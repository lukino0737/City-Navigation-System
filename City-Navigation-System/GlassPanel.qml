import QtQuick
import QtQuick.Effects

Item {
    id: root
    property int radius: 24
    property color glassColor: theme.panelColor
    property color borderColor: theme.panelBorder
    property int borderWidth: 1

    // blurSource: 指向需要被模糊的背景 Item（通常是 mapContainer）
    property Item blurSource: null

    // ── 坐标追踪 ────────────────────────────────────────────────────────────
    // 将 GlassPanel 的 (0,0) 映射到 blurSource 的坐标系中，得到截图偏移量。
    // 注意：mapToItem() 是 JS 函数调用，QML binding 引擎无法自动追踪
    // 其内部的坐标链依赖（父级位置、锚点解算等），必须用显式属性 + 轮询更新。
    property real _captureX: 0
    property real _captureY: 0

    function _refreshCapture() {
        if (!root.blurSource) return
        var pt = root.mapToItem(root.blurSource, 0, 0)
        _captureX = pt.x
        _captureY = pt.y
    }

    // 自身几何变化时立即刷新
    onXChanged:      _refreshCapture()
    onYChanged:      _refreshCapture()
    onWidthChanged:  _refreshCapture()
    onHeightChanged: _refreshCapture()

    // blurSource 赋值后延迟一帧再刷新，确保此时布局已完成解算
    onBlurSourceChanged: Qt.callLater(_refreshCapture)

    // 组件完成创建后延迟一帧刷新（此时父级锚点已解算完毕）
    Component.onCompleted: Qt.callLater(_refreshCapture)

    // 轮询 Timer：用于处理父级运动（如 RightPanel 滑入/滑出动画）期间的坐标变化
    // 仅在可见且有 blurSource 时运行，不增加额外开销
    Timer {
        interval: 16   // ~60fps
        running: root.visible && root.blurSource !== null
        repeat: true
        onTriggered: root._refreshCapture()
    }

    // ── 阴影层（最底层）────────────────────────────────────────────────────
    Rectangle {
        id: shadowBase
        anchors.fill: parent
        radius: root.radius
        color: "transparent"
        visible: false
    }
    MultiEffect {
        anchors.fill: shadowBase
        source: shadowBase
        shadowEnabled: true
        shadowBlur: 0.9
        shadowColor: Qt.rgba(0, 0, 0, 0.35)
        shadowVerticalOffset: 16
        shadowHorizontalOffset: 0
    }

    // ── 背景截图：使用 _captureX/_captureY 作为 binding 依赖 ───────────────
    // ShaderEffectSource 的 sourceRect 依赖 _captureX/_captureY，
    // 每当 Timer 刷新这两个属性，Qt 会自动重新截图，坐标始终精准。
    ShaderEffectSource {
        id: bgCapture
        width:  root.width
        height: root.height
        sourceItem: root.blurSource
        sourceRect: Qt.rect(root._captureX, root._captureY, root.width, root.height)
        live: true
        visible: false
        mipmap: false   // 禁用 mipmap，避免模糊边缘出现黑边
    }

    // ── 圆角 Mask（两步法关键）──────────────────────────────────────────────
    // Qt 6 MultiEffect.maskSource 只接受 ShaderEffectSource，不接受裸 Item。
    // 必须将 Rectangle 中转一次，否则圆角裁剪静默失效（退化为矩形裁剪）。
    Rectangle {
        id: maskShape
        width:  root.width
        height: root.height
        radius: root.radius
        color:  "white"
        visible: false
    }
    ShaderEffectSource {
        id: maskSES
        width:  root.width
        height: root.height
        sourceItem: maskShape
        live: false    // Mask 形状固定，无需每帧更新
        visible: false
    }

    // ── 模糊层：高斯模糊 + 圆角裁剪 ──────────────────────────────────────
    MultiEffect {
        anchors.fill: parent
        source: bgCapture
        visible: root.blurSource !== null

        blurEnabled: true
        blurMax: 48
        blur: 0.85              // 模糊强度 0.0 ~ 1.0

        maskEnabled: true
        maskSource: maskSES
        maskThresholdMin: 0.5
        maskSpreadAtMin: 0.0
    }

    // ── 着色层：Acrylic 色调 + 边框 + 顶部内侧高光 ─────────────────────
    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: root.glassColor
        border.color: root.borderColor
        border.width: root.borderWidth

        // 顶部内侧高光（模拟玻璃折射）
        Rectangle {
            anchors {
                top:   parent.top;  topMargin:   root.borderWidth
                left:  parent.left; leftMargin:  root.borderWidth
                right: parent.right; rightMargin: root.borderWidth
            }
            height: parent.height * 0.5
            radius: root.radius - root.borderWidth
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.07) }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }
    }
}
