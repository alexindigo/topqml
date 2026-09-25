// topqml minimal example — CPU %, per-core count, Mem used/total, process
// count, and a filterable process list. Read-only: kill/renice not exercised.
import QtQuick
import QtQuick.Controls
import Top

ApplicationWindow {
    id: root
    visible: true
    width: 420
    height: 600
    title: "topqml minimal"

    function gib(bytes) {
        return (bytes / 1073741824).toFixed(1) + " GiB"
    }

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text {
            text: "CPU " + CpuMonitor.usage + "%  (" + CpuMonitor.perCore.length + " cores)"
            font.pixelSize: 16
        }
        Text {
            text: "MEM " + root.gib(MemMonitor.used) + " / " + root.gib(MemMonitor.total)
            font.pixelSize: 16
        }
        Text {
            text: "Processes: " + ProcessModel.totalProcs
            font.pixelSize: 16
        }

        TextField {
            id: filterField
            width: parent.width
            placeholderText: "filter processes…"
        }

        ListView {
            width: parent.width
            height: parent.height - y
            clip: true
            model: ProcFilterProxyModel {
                sourceModel: ProcessModel
                filter: filterField.text
            }
            delegate: Text {
                required property string name
                required property double cpuPct
                text: name + "  " + cpuPct.toFixed(1) + "%"
                font.pixelSize: 12
            }
        }
    }
}
