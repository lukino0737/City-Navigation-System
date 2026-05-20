import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    visible: true 

    property Item blurTarget: null
    property var infoData: ({})

    function showInfo(info) {
        infoData = info
    }

    // Block mouse events from reaching map
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
    }

    GlassPanel {
        anchors.fill: parent
        radius: 24
        blurSource: root.blurTarget

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 32
            spacing: 24

            // Header
            ColumnLayout {
                spacing: 4
                Text {
                    text: {
                        if (infoData.type === "node") return i18n.t("detail.header.node")
                        if (infoData.type === "edge") return i18n.t("detail.header.edge")
                        if (infoData.type === "path") return i18n.t("detail.header.path")
                        return ""
                    }
                    color: theme.accentColor
                    font.pixelSize: 11
                    font.letterSpacing: 2
                    font.weight: Font.Bold
                }
                Text {
                    text: {
                        if (infoData.type === "node")
                            return i18n.t("detail.title.node", infoData.id || "—")
                        if (infoData.type === "edge")
                            return i18n.t("detail.title.edge", infoData.source || "—", infoData.target || "—")
                        if (infoData.type === "path") {
                            if (infoData.found) {
                                var sLabel = (infoData.startName && infoData.startName !== "") ? infoData.startName : ("#" + (infoData.startId || "—"))
                                var eLabel = (infoData.endName && infoData.endName !== "") ? infoData.endName : ("#" + (infoData.endId || "—"))
                                return i18n.t("detail.title.path.ok", sLabel, eLabel)
                            }
                            else
                                return i18n.t("detail.title.path.fail")
                        }
                        return ""
                    }
                    color: (infoData.type === "path" && !infoData.found) ? "#f87171" : theme.textColor
                    font.pixelSize: 22
                    font.weight: Font.Light
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: theme.dividerColor }

            // Scrollable property list
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    Repeater {
                        model: {
                            var rows = []
                            if (infoData.type === "node") {
                                rows.push({label: i18n.t("detail.node.id"),    value: infoData.id || "—"})
                                var nodeName = infoData.name
                                if (nodeName && nodeName !== "") {
                                    rows.push({label: i18n.t("detail.node.name"), value: nodeName})
                                }
                                rows.push({label: i18n.t("detail.node.x"), value: (infoData.x !== undefined ? infoData.x.toFixed(2) : "—")})
                                rows.push({label: i18n.t("detail.node.y"), value: (infoData.y !== undefined ? infoData.y.toFixed(2) : "—")})
                                rows.push({label: i18n.t("detail.node.degree"),     value: infoData.degree !== undefined ? infoData.degree : "—"})
                            } else if (infoData.type === "edge") {
                                rows.push({label: i18n.t("detail.edge.source"),   value: infoData.source || "—"})
                                rows.push({label: i18n.t("detail.edge.target"),   value: infoData.target || "—"})
                                rows.push({label: i18n.t("detail.edge.capacity"),    value: infoData.capacity || "—"})
                                rows.push({label: i18n.t("detail.edge.currentCars"), value: infoData.currentCars !== undefined ? infoData.currentCars : "—"})
                                rows.push({label: i18n.t("detail.edge.length"),      value: infoData.length !== undefined ? infoData.length.toFixed(1) : "—"})
                            } else if (infoData.type === "path") {
                                if (infoData.found) {
                                    rows.push({label: i18n.t("detail.path.start"),   value: infoData.startId || "—"})
                                    var startName = infoData.startName
                                    if (startName && startName !== "") {
                                        rows.push({label: i18n.t("detail.path.startName"), value: startName})
                                    }
                                    rows.push({label: i18n.t("detail.path.end"),     value: infoData.endId || "—"})
                                    var endName = infoData.endName
                                    if (endName && endName !== "") {
                                        rows.push({label: i18n.t("detail.path.endName"), value: endName})
                                    }
                                    var formatTime = function(mins) {
                                        if (mins === undefined) return "—"
                                        var h = Math.floor(mins / 60)
                                        var m = Math.round(mins % 60)
                                        if (h > 0) {
                                            return h + i18n.t("time.hours") + m + i18n.t("time.minutes")
                                        }
                                        return m + i18n.t("time.minutes")
                                    }
                                    rows.push({label: i18n.t("detail.path.cost"),   value: formatTime(infoData.totalCost)})
                                    rows.push({label: i18n.t("detail.path.hops"),         value: infoData.hopCount !== undefined ? infoData.hopCount : "—"})
                                    rows.push({label: i18n.t("detail.path.nodes"), value: infoData.nodeCount !== undefined ? infoData.nodeCount : "—"})
                                    rows.push({label: i18n.t("detail.path.time"), value: infoData.timeMs !== undefined ? infoData.timeMs.toFixed(2) + i18n.t("detail.path.timeUnit") : "—"})
                                    // List named waypoints in the path (vertical)
                                    var namedWaypoints = []
                                    if (infoData.pathNodeNames && infoData.pathNodeIds) {
                                        for (var pni = 0; pni < infoData.pathNodeNames.length; pni++) {
                                            var pname = infoData.pathNodeNames[pni]
                                            if (pname && pname !== "") {
                                                namedWaypoints.push({num: infoData.pathNodeIds[pni], name: pname})
                                            }
                                        }
                                    }
                                    if (namedWaypoints.length > 0) {
                                        rows.push({label: i18n.t("detail.path.namedNodes"), value: namedWaypoints.length})
                                        for (var wi = 0; wi < namedWaypoints.length; wi++) {
                                            rows.push({label: "#" + namedWaypoints[wi].num, value: namedWaypoints[wi].name})
                                        }
                                    }
                                } else {
                                    rows.push({label: i18n.t("detail.path.start"), value: infoData.startId || "—"})
                                    rows.push({label: i18n.t("detail.path.end"),   value: infoData.endId || "—"})
                                    rows.push({label: i18n.t("detail.path.result"),     value: i18n.t("detail.path.noRoute")})
                                }
                            }
                            return rows
                        }

                        delegate: ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: modelData.label
                                color: theme.subTextColor
                                font.pixelSize: 12
                            }
                            Text {
                                text: modelData.value
                                color: theme.textColor
                                font.pixelSize: 15
                                font.weight: Font.Medium
                            }
                            Item { height: 4 }
                        }
                    }
                    
                    // Spacer at the bottom to avoid being cut off by external switcher
                    Item { Layout.preferredHeight: 64 }
                }
            }
        }
    }
}
