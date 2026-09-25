/*
 * topqml — TopConfig singleton.
 *
 * vostop's Settings with persistence removed: collectors keep the same
 * thread-safe, string-keyed static API (getB/getS/getI + setB/setS/setI)
 * and the same per-key typed C++ accessors, over a mutex-guarded cache
 * backed by the defaults table lifted verbatim from vostop's
 * src/backend/Settings.cpp. Typed QML properties cover the consumer-facing
 * subset; every other key stays reachable through the static API and the
 * C++ accessors. The library persists nothing — policy belongs to the
 * consumer.
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVariantHash>
#include <QReadWriteLock>
#include <QtQmlIntegration/qqmlintegration.h>
QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

//? Defaults table, lifted verbatim from vostop's SETTINGS_DSL — all keys,
//? including ones only vostop's app layer reads; identical tables keep
//? divergence cherry-picking trivial.
#define TOPCONFIG_DSL \
	X(bool, showSwap, "show_swap", true) \
	X(bool, swapDisk, "swap_disk", true) \
	X(bool, showCpuFreq, "show_cpu_freq", true) \
	X(QString, freqMode, "freq_mode", "first") \
	X(QString, cpuCoreMap, "cpu_core_map", "") \
	X(bool, zfsArcCached, "zfs_arc_cached", true) \
	X(bool, checkTemp, "check_temp", false) \
	X(bool, showCoretemp, "show_coretemp", true) \
	X(QString, cpuSensor, "cpu_sensor", "Auto") \
	X(bool, showBattery, "show_battery", false) \
	X(QString, selectedBattery, "selected_battery", "Auto") \
	X(bool, showCpuWatts, "show_cpu_watts", false) \
	X(bool, showDisks, "show_disks", true) \
	X(int, pollIntervalMs, "poll_interval_ms", 1000) \
	X(QString, procSorting, "proc_sorting", "cpu lazy") \
	X(bool, procReversed, "proc_reversed", false) \
	X(QString, procFilter, "proc_filter", "") \
	X(bool, procPerCore, "proc_per_core", false) \
	X(bool, procFilterKernel, "proc_filter_kernel", false) \
	X(bool, pauseProcList, "pause_proc_list", false) \
	X(bool, keepDeadProcUsage, "keep_dead_proc_usage", false) \
	X(bool, procInfoSmaps, "proc_info_smaps", false) \
	X(QString, disksFilter, "disks_filter", "") \
	X(bool, diskFreePriv, "disk_free_priv", false) \
	X(bool, useFstab, "use_fstab", true) \
	X(bool, onlyPhysical, "only_physical", true) \
	X(bool, zfsHideDatasets, "zfs_hide_datasets", false) \
	X(QString, netIface, "net_iface", "") \
	X(bool, netSync, "net_sync", true) \
	X(bool, netAuto, "net_auto", true) \
	X(QString, shownGpus, "shown_gpus", "Auto") \
	X(bool, nvmlMeasurePcieSpeeds, "nvml_measure_pcie_speeds", false) \
	X(bool, rsmiMeasurePcieSpeeds, "rsmi_measure_pcie_speeds", false) \
	X(bool, groupCollapsedApps, "group_collapsed_apps", false) \
	X(bool, groupCollapsedBackground, "group_collapsed_background", false) \
	X(bool, groupCollapsedSystem, "group_collapsed_system", false) \
	X(bool, procIoReads, "proc_io_reads", true) \
	X(bool, gpuFdinfoWalk, "gpu_fdinfo_walk", true)

//? Consumer-facing typed subset exposed to QML (property names mirrored
//? from vostop's Settings.h); all other keys stay reachable via the static
//? API and the C++ accessors.
#define TOPCONFIG_QML_DSL \
	X(int, pollIntervalMs, "poll_interval_ms", 1000) \
	X(bool, procIoReads, "proc_io_reads", true) \
	X(bool, gpuFdinfoWalk, "gpu_fdinfo_walk", true) \
	X(bool, showBattery, "show_battery", false) \
	X(bool, showCpuWatts, "show_cpu_watts", false) \
	X(QString, netIface, "net_iface", "") \
	X(QString, procSorting, "proc_sorting", "cpu lazy") \
	X(bool, procReversed, "proc_reversed", false) \
	X(QString, procFilter, "proc_filter", "") \
	X(bool, procPerCore, "proc_per_core", false) \
	X(bool, procFilterKernel, "proc_filter_kernel", false)

class TopConfig : public QObject {
	Q_OBJECT
	QML_SINGLETON
	QML_ELEMENT
	//? QML properties: consumer-facing subset only — X(type, Name, "key", default)
#define X(type, Name, key, def) \
	Q_PROPERTY(type Name READ Name WRITE set_##Name NOTIFY Name##Changed FINAL)
	TOPCONFIG_QML_DSL
#undef X

public:
	static TopConfig* instance();

	//* QML singleton provider — returns the same instance the C++ side uses
	static TopConfig* create(QQmlEngine* engine, QJSEngine* jsEngine);

	//? Thread-safe collector-side accessors (vostop Settings signatures)
	static bool getB(const QString& key);
	static QString getS(const QString& key);
	static int getI(const QString& key);
	static void setB(const QString& key, bool v);
	static void setS(const QString& key, const QString& v);
	static void setI(const QString& key, int v);

	//? Per-key typed C++ accessors for the full table (vostop Settings
	//? shape — extraction-set call sites stay verbatim after the rename)
#define X(type, Name, key, def) \
	type Name() const { \
		QReadLocker lock(&m_lock); \
		return m_cache.value(QStringLiteral(key)).value<type>(); \
	} \
	void set_##Name(const type& v);
	TOPCONFIG_DSL
#undef X

signals:
#define X(type, Name, key, def) void Name##Changed();
	TOPCONFIG_DSL
#undef X

private:
	explicit TopConfig(QObject* parent = nullptr);

	QVariantHash m_cache;
	mutable QReadWriteLock m_lock;
};
