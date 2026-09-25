# topqml

A Qt6/QML module (URI `Top`) providing system-monitoring collectors with QML
bindings: CPU, memory, disks, network, GPU, sensors, and the full process
table — for any QML app. The collector core is derived from
[btop](https://github.com/aristocratos/btop) and extracted from
[vostop](https://github.com/alexindigo/vostop) into a standalone,
policy-free library (no persistence, no theming — consumers own both).

One shared collector engine (QThread + poll timer) starts when the first
singleton is touched and feeds every widget in the process. No setup code:

```qml
import QtQuick
import Top

Text { text: "CPU " + CpuMonitor.usage + "%" }
```

## QML API

Singletons: `CpuMonitor`, `MemMonitor`, `DiskMonitor`, `NetMonitor`,
`GpuMonitor`, `SensorsMonitor`, `ProcessModel`, `TopConfig`.
Instantiable: `ProcFilterProxyModel`.

### Bar widget (CPU% + sparkline + MEM%)

```qml
import QtQuick
import Top

Rectangle {
    id: root
    implicitWidth: 220; implicitHeight: 28
    color: "#cc000000"; radius: 6

    Row {
        anchors.centerIn: parent
        spacing: 10

        Text {                            // "CPU 12%"
            text: "CPU " + CpuMonitor.usage + "%"
            color: CpuMonitor.usage > 85 ? "#ff6b6b" : "#e6e6e6"
            font.pixelSize: 12
        }

        Canvas {                          // 60-sample sparkline off
            id: spark                     // CpuMonitor.history
            width: 60; height: 16
            anchors.verticalCenter: parent.verticalCenter
            property var pts: CpuMonitor.history
            onPtsChanged: requestPaint()
            onPaint: {
                const ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                if (pts.length < 2) return
                ctx.strokeStyle = "#4dd0a6"; ctx.lineWidth = 1
                ctx.beginPath()
                for (let i = 0; i < pts.length; i++) {
                    const x = i * width / (pts.length - 1)
                    const y = height - (pts[i] / 100) * height
                    i ? ctx.lineTo(x, y) : ctx.moveTo(x, y)
                }
                ctx.stroke()
            }
        }

        Text {                            // "MEM 41%"
            text: "MEM " + Math.round(100 * MemMonitor.used
                                      / Math.max(1, MemMonitor.total)) + "%"
            color: "#e6e6e6"
            font.pixelSize: 12
        }
    }
}
```

### Optional config + process list

```qml
import QtQuick
import QtQuick.Controls
import Top

Component.onCompleted: TopConfig.pollIntervalMs = 2000   // bar doesn't need 1 Hz

ListView {
    model: ProcFilterProxyModel { sourceModel: ProcessModel; filter: "qs" }
    delegate: Text { text: model.name + "  " + model.cpuPct + "%" }
}
```

### Selected properties

- `CpuMonitor`: `usage` (int %), `perCore`, `coreHistories`, `history`,
  `freqText`, `load1/5/15`, `cpuName`, `handles`, `uptimeSec`, pressure set
- `MemMonitor`: `total`, `used`, `free`, `available`, `cached`, `swapTotal`,
  `swapUsed`, `hasSwap`, commit/kernel stats, `history`, `swapHistory`,
  pressure set
- `DiskMonitor`: `mounts` (QVariantList), io pressure set
- `NetMonitor`: `iface` (writable, in-memory only), link speed, rates
- `ProcessModel`: table model — roles `pid`, `name`, `cmd`, `cpuPct`,
  `memBytes`, `user`, `threads`, `state`, `ppid`, `ioReadRate`,
  `ioWriteRate`, `ioKnown`, `category`; `Q_INVOKABLE` `killProcess(pid,
  signal)`, `renice(pid, priority)`, `requestOpenFiles(pid)`
- `ProcFilterProxyModel`: `filter` (writable string), `sortBy(column,
  descending)`
- `TopConfig`: typed properties for the consumer-facing subset
  (`pollIntervalMs`, `procIoReads`, `gpuFdinfoWalk`, `showBattery`,
  `showCpuWatts`, `netIface`, and the proc sorting/filter group); every
  other collector key is reachable from C++ via the static
  `TopConfig::getB/getS/getI` + `setB/setS/setI` API. Nothing is persisted
  — policy belongs to the consumer.

## Requirements

Linux (collectors read `/proc` + `/sys`), Qt ≥ 6.5 (`Core`, `Qml`;
`Quick` for the example app), CMake ≥ 3.16, a C++20 compiler.

## Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Install

```sh
cmake --install build --prefix /usr
```

The module installs to `<prefix>/lib/qt6/qml/Top` — after install,
`import Top` works with no environment variables.

## Example app

`examples/minimal` is a small read-only monitor (CPU %, per-core count,
Mem used/total, process count, filterable process list) built as its own
CMake project:

```sh
cmake -B examples/minimal/build -S examples/minimal
cmake --build examples/minimal/build
# against the build tree:
QML_IMPORT_PATH=$PWD/build examples/minimal/build/minimal
```

## Testing

```sh
ctest --test-dir build --output-on-failure
```

`test/smoke.qml` loads the module offscreen, lets the engine tick, and
asserts the singletons populate (`CpuMonitor.perCore` non-empty,
`MemMonitor.total > 0`, `ProcessModel.totalProcs > 0`) and that
`ProcFilterProxyModel.filter` narrows the table.

## License

GPL-3.0-or-later. See [LICENSE](LICENSE); btop-derived collector portions
carry their Apache-2.0 attribution in [THIRD-PARTY-NOTICES](THIRD-PARTY-NOTICES).
