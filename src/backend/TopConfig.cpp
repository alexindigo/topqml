/*
 * topqml — TopConfig singleton (implementation).
 */
#include "TopConfig.h"

#include <QQmlEngine>

TopConfig* TopConfig::instance() {
	static TopConfig inst;
	return &inst;
}

TopConfig* TopConfig::create(QQmlEngine* engine, QJSEngine* jsEngine) {
	Q_UNUSED(engine);
	Q_UNUSED(jsEngine);
	TopConfig* obj = instance();
	QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
	return obj;
}

TopConfig::TopConfig(QObject* parent)
	: QObject(parent) {
	//? btop defaults via vostop; no persistence — consumers own policy
	QWriteLocker lock(&m_lock);
#define X(type, Name, key, def) \
	m_cache.insert(QStringLiteral(key), QVariant::fromValue<type>(def));
	TOPCONFIG_DSL
#undef X
}

bool TopConfig::getB(const QString& key) {
	TopConfig* c = instance();
	QReadLocker lock(&c->m_lock);
	return c->m_cache.value(key).toBool();
}

QString TopConfig::getS(const QString& key) {
	TopConfig* c = instance();
	QReadLocker lock(&c->m_lock);
	return c->m_cache.value(key).toString();
}

int TopConfig::getI(const QString& key) {
	TopConfig* c = instance();
	QReadLocker lock(&c->m_lock);
	return c->m_cache.value(key).toInt();
}

void TopConfig::setB(const QString& key, bool v) {
	TopConfig* c = instance();
	QWriteLocker lock(&c->m_lock);
	c->m_cache.insert(key, QVariant::fromValue(v));
}

void TopConfig::setS(const QString& key, const QString& v) {
	TopConfig* c = instance();
	QWriteLocker lock(&c->m_lock);
	c->m_cache.insert(key, QVariant::fromValue(v));
}

void TopConfig::setI(const QString& key, int v) {
	TopConfig* c = instance();
	QWriteLocker lock(&c->m_lock);
	c->m_cache.insert(key, QVariant::fromValue(v));
}

#define X(type, Name, key, def) \
	void TopConfig::set_##Name(const type& v) { \
		{ \
			QWriteLocker lock(&m_lock); \
			if (m_cache.value(QStringLiteral(key)).value<type>() == v) return; \
			m_cache.insert(QStringLiteral(key), QVariant::fromValue<type>(v)); \
		} \
		emit Name##Changed(); \
	}
TOPCONFIG_DSL
#undef X
