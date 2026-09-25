/*
 * topqml — TopEngine (implementation).
 */
#include "TopEngine.h"

#include "CollectorWorker.h"
#include "Monitors.h"
#include "DiskNetMonitors.h"
#include "GpuSensorsMonitors.h"
#include "ProcessModel.h"
#include "TopConfig.h"

#include <QThread>
#include <QTimer>

#include <mutex>

namespace {

//* Engine state, constructed on the deferred wiring turn (GUI thread) and
//* destroyed at static-destruction time — before the singleton statics,
//* since it is constructed after them.
struct Engine {
	QThread thread;
	CollectorWorker* worker = nullptr;

	Engine() {
		worker = new CollectorWorker;
		worker->moveToThread(&thread);
		QObject::connect(&thread, &QThread::started, worker, &CollectorWorker::start);
		QObject::connect(&thread, &QThread::finished, worker, &QObject::deleteLater);
		//? Queued (auto) connections: worker emits from the worker thread → GUI-thread monitors
		QObject::connect(worker, &CollectorWorker::cpuUpdated, &CpuMonitor::instance(), &CpuMonitor::update);
		QObject::connect(worker, &CollectorWorker::memUpdated, &MemMonitor::instance(), &MemMonitor::update);
		QObject::connect(worker, &CollectorWorker::procUpdated, &ProcessModel::instance(), &ProcessModel::update);
		QObject::connect(worker, &CollectorWorker::procDetailUpdated, &ProcessModel::instance(), &ProcessModel::detailUpdated);
		QObject::connect(worker, &CollectorWorker::openFilesUpdated, &ProcessModel::instance(), &ProcessModel::openFilesUpdated);
		QObject::connect(worker, &CollectorWorker::diskUpdated, &DiskMonitor::instance(), &DiskMonitor::update);
		QObject::connect(worker, &CollectorWorker::netUpdated, &NetMonitor::instance(), &NetMonitor::update);
		QObject::connect(worker, &CollectorWorker::gpuUpdated, &GpuMonitor::instance(), &GpuMonitor::update);
		QObject::connect(worker, &CollectorWorker::sensorsUpdated, &SensorsMonitor::instance(), &SensorsMonitor::update);
		//? Poll-interval setting applies live (queued into the worker thread)
		QObject::connect(TopConfig::instance(), &TopConfig::pollIntervalMsChanged,
			worker, &CollectorWorker::applyInterval, Qt::QueuedConnection);
		thread.start();
	}

	~Engine() {
		QMetaObject::invokeMethod(worker, &CollectorWorker::stop, Qt::BlockingQueuedConnection);
		thread.quit();
		thread.wait();
	}
};

Engine& engine() {
	static Engine inst;
	return inst;
}

} // namespace

void TopEngine::ensureStarted() {
	static std::once_flag once;
	std::call_once(once, [] {
		qRegisterMetaType<CpuSnapshot>("CpuSnapshot");
		qRegisterMetaType<MemSnapshot>("MemSnapshot");
		qRegisterMetaType<ProcSnapshot>("ProcSnapshot");
		qRegisterMetaType<ProcDetailSnapshot>("ProcDetailSnapshot");
		qRegisterMetaType<OpenFilesSnapshot>("OpenFilesSnapshot");
		qRegisterMetaType<DiskSnapshot>("DiskSnapshot");
		qRegisterMetaType<NetSnapshot>("NetSnapshot");
		qRegisterMetaType<GpuSnapshot>("GpuSnapshot");
		qRegisterMetaType<SensorsSnapshot>("SensorsSnapshot");
		//? Deferred: the triggering singleton's ctor is still on the stack;
		//? touching its instance() now would be recursive static init (UB).
		QTimer::singleShot(0, [] { (void)engine(); });
	});
}
