// topqml smoke test — loads the Top module, lets the collector engine tick,
// asserts the singletons populate and the filter proxy narrows.
// Run via ctest, or: QML_IMPORT_PATH=<build> QT_QPA_PLATFORM=offscreen qml6 test/smoke.qml
import QtQuick
import Top

Item {
    ProcFilterProxyModel {
        id: proxy
        sourceModel: ProcessModel
    }

    Component.onCompleted: {
        console.warn("=== Top Smoke Test ===")
        console.warn("TopConfig.pollIntervalMs:", TopConfig.pollIntervalMs)
        probeTimer.start()
    }

    Timer {
        id: probeTimer
        interval: 2500 // engine ticks at 1 Hz (default) → ≥2 snapshots
        onTriggered: {
            var failures = []

            console.warn("CPU usage:", CpuMonitor.usage, "% cores:", CpuMonitor.perCore.length)
            console.warn("Mem used/total:", MemMonitor.used, "/", MemMonitor.total)
            console.warn("Processes:", ProcessModel.totalProcs)

            if (TopConfig.pollIntervalMs !== 1000)
                failures.push("TopConfig default pollIntervalMs != 1000")
            if (CpuMonitor.perCore.length < 1)
                failures.push("CpuMonitor.perCore empty — engine not delivering")
            if (CpuMonitor.usage < 0 || CpuMonitor.usage > 100)
                failures.push("CpuMonitor.usage out of range: " + CpuMonitor.usage)
            if (MemMonitor.total <= 0)
                failures.push("MemMonitor.total not populated")
            if (ProcessModel.totalProcs < 1)
                failures.push("ProcessModel.totalProcs not populated")

            var netRows = NetIfacesModel.rowCount()
            console.warn("NetIfacesModel rows:", netRows)
            if (netRows < 1)
                failures.push("NetIfacesModel.rowCount < 1")
            for (var r = 0; r < netRows; r++) {
                var nidx = NetIfacesModel.index(r, 0)
                console.warn("net", NetIfacesModel.data(nidx, NetIfacesModel.NameRole),
                             "down", NetIfacesModel.data(nidx, NetIfacesModel.DownTotalRole),
                             "up", NetIfacesModel.data(nidx, NetIfacesModel.UpTotalRole))
            }

            var total = proxy.rowCount()
            proxy.filter = "definitely-no-such-process-name-xyz"
            var filtered = proxy.rowCount()
            console.warn("Proxy rows:", total, "→ filtered:", filtered)
            if (total < 1)
                failures.push("proxy rowCount empty")
            if (total > 0 && filtered >= total)
                failures.push("proxy filter did not narrow the table")

            if (failures.length === 0) {
                console.warn("=== Smoke Test PASS ===")
                Qt.exit(0)
            } else {
                for (var i = 0; i < failures.length; i++)
                    console.warn("FAIL:", failures[i])
                console.warn("=== Smoke Test FAIL ===")
                Qt.exit(1)
            }
        }
    }
}
