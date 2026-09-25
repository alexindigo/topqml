// topqml minimal example — verification vehicle for the Top module.
// Run with QT_QML_IMPORT_PATH pointing at the topqml build directory.
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[]) {
	QGuiApplication app(argc, argv);
	QQmlApplicationEngine engine;
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
		&app, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
	engine.loadFromModule("MinimalExample", "Main");
	return app.exec();
}
