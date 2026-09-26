/*
 * topqml — DiskMonitor / NetIfacesModel (implementation).
 */
#include "DiskNetMonitors.h"

#include "TopEngine.h"

#include <QQmlEngine>

DiskMonitor::DiskMonitor(QObject* parent) : QObject(parent) { TopEngine::ensureStarted(); }

DiskMonitor& DiskMonitor::instance() {
	static DiskMonitor inst;
	return inst;
}

DiskMonitor* DiskMonitor::create(QQmlEngine* engine, QJSEngine* jsEngine) {
	Q_UNUSED(engine);
	Q_UNUSED(jsEngine);
	DiskMonitor* obj = &instance();
	QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
	return obj;
}

QList<double> DiskMonitor::ioPressureSome() const {
	return {m_ioPressure.some[0], m_ioPressure.some[1], m_ioPressure.some[2]};
}

QList<double> DiskMonitor::ioPressureFull() const {
	return {m_ioPressure.full[0], m_ioPressure.full[1], m_ioPressure.full[2]};
}

void DiskMonitor::update(const DiskSnapshot& snapshot) {
	//? Rebuild the mount list (slow-changing data; stolen per-mount stats)
	QVariantList newList;
	for (const auto& m : snapshot.mounts) {
		QVariantMap row;
		row["dev"] = m.dev;
		row["name"] = m.name;
		row["fstype"] = m.fstype;
		row["mountpoint"] = m.mountpoint;
		row["total"] = m.total;
		row["used"] = m.used;
		row["free"] = m.free;
		row["usedPercent"] = m.usedPercent;
		row["freePercent"] = m.freePercent;
		row["ioRead"] = m.ioRead;
		row["ioWrite"] = m.ioWrite;
		row["ioActivity"] = m.ioActivity;
		newList.append(row);
	}
	if (newList != m_mounts) {
		m_mounts = newList;
		emit mountsChanged();
	}

	if (m_ioPressure.valid != snapshot.ioPressureValid
		or m_ioPressure.some[0] != snapshot.ioPressureSome[0]
		or m_ioPressure.some[1] != snapshot.ioPressureSome[1]
		or m_ioPressure.some[2] != snapshot.ioPressureSome[2]
		or m_ioPressure.full[0] != snapshot.ioPressureFull[0]
		or m_ioPressure.full[1] != snapshot.ioPressureFull[1]
		or m_ioPressure.full[2] != snapshot.ioPressureFull[2]) {
		m_ioPressure.some[0] = snapshot.ioPressureSome[0];
		m_ioPressure.some[1] = snapshot.ioPressureSome[1];
		m_ioPressure.some[2] = snapshot.ioPressureSome[2];
		m_ioPressure.full[0] = snapshot.ioPressureFull[0];
		m_ioPressure.full[1] = snapshot.ioPressureFull[1];
		m_ioPressure.full[2] = snapshot.ioPressureFull[2];
		m_ioPressure.hasFull = snapshot.ioPressureHasFull;
		m_ioPressure.valid = snapshot.ioPressureValid;
		m_ioPressureHistory = snapshot.ioPressureHistory;
		emit ioPressureChanged();
	}
}

NetIfacesModel::NetIfacesModel(QObject* parent) : QAbstractListModel(parent) { TopEngine::ensureStarted(); }

NetIfacesModel& NetIfacesModel::instance() {
	static NetIfacesModel inst;
	return inst;
}

NetIfacesModel* NetIfacesModel::create(QQmlEngine* engine, QJSEngine* jsEngine) {
	Q_UNUSED(engine);
	Q_UNUSED(jsEngine);
	NetIfacesModel* obj = &instance();
	QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
	return obj;
}

int NetIfacesModel::rowCount(const QModelIndex& parent) const {
	return parent.isValid() ? 0 : static_cast<int>(m_ifaces.size());
}

QVariant NetIfacesModel::data(const QModelIndex& index, int role) const {
	if (not index.isValid() or index.row() < 0 or index.row() >= static_cast<int>(m_ifaces.size()))
		return {};
	const auto& iface = m_ifaces.at(index.row());
	switch (role) {
		case NameRole: return iface.name;
		case Ipv4Role: return iface.ipv4;
		case Ipv6Role: return iface.ipv6;
		case ConnectedRole: return iface.connected;
		case LinkSpeedRole: return iface.linkSpeed;
		case DownSpeedRole: return iface.downSpeed;
		case UpSpeedRole: return iface.upSpeed;
		case DownTotalRole: return iface.downTotal;
		case UpTotalRole: return iface.upTotal;
		case DownHistoryRole: return QVariant::fromValue(iface.downHistory);
		case UpHistoryRole: return QVariant::fromValue(iface.upHistory);
	}
	return {};
}

QHash<int, QByteArray> NetIfacesModel::roleNames() const {
	return {
		{ NameRole, "name" },
		{ Ipv4Role, "ipv4" },
		{ Ipv6Role, "ipv6" },
		{ ConnectedRole, "connected" },
		{ LinkSpeedRole, "linkSpeed" },
		{ DownSpeedRole, "downSpeed" },
		{ UpSpeedRole, "upSpeed" },
		{ DownTotalRole, "downTotal" },
		{ UpTotalRole, "upTotal" },
		{ DownHistoryRole, "downHistory" },
		{ UpHistoryRole, "upHistory" },
	};
}

void NetIfacesModel::update(const NetSnapshot& snapshot) {
	//? Full reset per tick — interfaces number in the single digits and the
	//? snapshot replaces every row's stats anyway; a row-diff is a future
	//? optimization, deliberately not done ("don't do too much on top of it").
	beginResetModel();
	m_ifaces = snapshot.ifaces;
	endResetModel();
}