# Changelog

## [0.1.1] — 2026-09-24

### Feature

topqml becomes the family's first C++-consumable library: the public
backend headers install to `include/topqml/`, the CMake export is
completed (`topqml::topqml` target, `topqmlConfig.cmake` +
version file), and Qt6::Core/Qml move to the public link interface so
`find_package(topqml)` consumers compile and link against the installed
package. `ProcFilterProxyModel` gains a static `matchesFilterRow`
wrapper over the btop matcher — the one collector symbol vostop's
`ProcGrouped` calls directly. No behavior changes; the QML surface is
identical. Rides along: the `TopConfig.h` comment now attributes the
defaults table to `Settings.h` (extraction addendum item B1).

- Ship C++ headers and CMake package config (head commit of this release)

## [0.1.0] — 2026-09-24

### Feature

Initial release: vostop's system-monitoring collector stack and QML
bindings extracted into a standalone, reusable Qt6/QML module (URI
`Top`), so any QML app can bind to CPU, memory, disk, network, GPU,
sensor, and process-table singletons with zero setup code. The collector
core is btop-derived and reads `/proc` + `/sys` on a worker thread; the
library is policy-free — a `TopConfig` singleton replaces vostop's
`Settings` coupling (same string-keyed API over the btop defaults table,
typed properties for the consumer-facing subset, no persistence), and a
`TopEngine` bootstrap starts the shared collector engine when the first
data singleton is touched. A minimal read-only example app (CPU %,
memory, process count, filterable process list) ships as the
verification vehicle and consumer reference.

- Import btop-derived collectors and QML bindings from vostop (`25148da`)
- Minimal example app (`1ab1365`)

### Docs

The README documents the module for consumers: bar-widget and
process-list QML examples, the singleton and property surface, build and
install instructions, and the example app. Alongside it, an offscreen
QML smoke test asserts the engine self-starts, the singletons populate,
and the filter proxy narrows — wired into CTest so it runs everywhere
the module builds.

- Readme and offscreen smoke test (`92f3e1a`)

### CI

GitHub Actions workflow on an Arch Linux container: dependency install,
Release build, qmllint over the QML files, and the CTest smoke test.
Every push to `master` and every pull request runs the full gate.

- Arch-container build, qmllint, ctest (`4763dcd`)
