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
                    text: infoData.type === "node" ? "NODE DETAILS" : "EDGE DETAILS"
                    color: theme.accentColor
                    font.pixelSize: 11
                    font.letterSpacing: 2
                    font.weight: Font.Bold
                }
                Text {
                    text: infoData.type === "node"
                        ? "Intersection #" + (infoData.id || "—")
                        : "Connection " + (infoData.source || "—") + " ↔ " + (infoData.target || "—")
                    color: theme.textColor
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
                                rows.push({label: "Node ID",    value: infoData.id || "—"})
                                rows.push({label: "Position X", value: (infoData.x !== undefined ? infoData.x.toFixed(2) : "—")})
                                rows.push({label: "Position Y", value: (infoData.y !== undefined ? infoData.y.toFixed(2) : "—")})
                                rows.push({label: "Degree",     value: infoData.degree !== undefined ? infoData.degree : "—"})
                            } else if (infoData.type === "edge") {
                                rows.push({label: "Source ID",   value: infoData.source || "—"})
                                rows.push({label: "Target ID",   value: infoData.target || "—"})
                                rows.push({label: "Capacity",    value: infoData.capacity || "—"})
                                rows.push({label: "Length",      value: infoData.length !== undefined ? infoData.length.toFixed(1) : "—"})
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
