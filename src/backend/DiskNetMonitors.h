/*
 * topqml — DiskMonitor / NetIfacesModel: GUI-thread holders fed by the
 * worker's disk/net collectors. NetIfacesModel exposes every interface the
 * collector tracks — the library holds no selection; consumers decide what
 * to display.
 */
#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QVector>
#include <QtQmlIntegration/qqmlintegration.h>

#include "snapshots.h"

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

class DiskMonitor : public QObject {
	Q_OBJECT
	QML_SINGLETON
	QML_ELEMENT
	//? Mount rows: dev/mount/fstype/used/total/ioRead/ioWrite
	Q_PROPERTY(QVariantList mounts READ mounts NOTIFY mountsChanged FINAL)
	//? io PSI (parity addition; valid=false → UI hides pressure rows)
	Q_PROPERTY(bool ioPressureValid READ ioPressureValid NOTIFY ioPressureChanged FINAL)
	Q_PROPERTY(QList<double> ioPressureSome READ ioPressureSome NOTIFY ioPressureChanged FINAL)
	Q_PROPERTY(QList<double> ioPressureFull READ ioPressureFull NOTIFY ioPressureChanged FINAL)
	Q_PROPERTY(QList<double> ioPressureHistory READ ioPressureHistory NOTIFY ioPressureChanged FINAL)

public:
private:
	explicit DiskMonitor(QObject* parent = nullptr);

public:

	static DiskMonitor& instance();
	static DiskMonitor* create(QQmlEngine* engine, QJSEngine* jsEngine);

	QVariantList mounts() const { return m_mounts; }
	bool ioPressureValid() const { return m_ioPressure.valid; }
	QList<double> ioPressureSome() const;
	QList<double> ioPressureFull() const;
	QList<double> ioPressureHistory() const { return m_ioPressureHistory; }

public slots:
	void update(const DiskSnapshot& snapshot);

signals:
	void mountsChanged();
	void ioPressureChanged();

private:
	QVariantList m_mounts;
	QList<double> m_ioPressureHistory;
	PressureSnapshot m_ioPressure;
};

//* One row per interface the collector tracks (current_net), roles 1:1 with
//* net_info. No selection, no aggregate — consumers decide what to display.
class NetIfacesModel : public QAbstractListModel {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

public:
	enum Roles {
		NameRole = Qt::UserRole + 1,
		Ipv4Role,
		Ipv6Role,
		ConnectedRole,
		LinkSpeedRole,
		DownSpeedRole,
		UpSpeedRole,
		DownTotalRole,
		UpTotalRole,
		DownHistoryRole,
		UpHistoryRole,
	};
	Q_ENUM(Roles)

private:
	explicit NetIfacesModel(QObject* parent = nullptr);

public:

	static NetIfacesModel& instance();
	static NetIfacesModel* create(QQmlEngine* engine, QJSEngine* jsEngine);

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

public slots:
	void update(const NetSnapshot& snapshot);

private:
	QVector<NetSnapshot::Iface> m_ifaces;
};