/*
 * topqml — TopEngine: shared collector-engine bootstrap.
 *
 * The library-side home of the wiring vostop keeps in main.cpp: snapshot
 * metatypes, the CollectorWorker QThread, and the queued snapshot-signal
 * connections into the seven data singletons. Started lazily — every data
 * singleton ctor calls ensureStarted(), so the engine spins up when the
 * first singleton is touched and consumers write no setup code.
 */
#pragma once

namespace TopEngine {

//* Idempotent: registers metatypes on first call and posts the worker
//* wiring to the next GUI-thread turn (deferred so the singleton ctor that
//* triggered it can finish — re-entering instance() mid-ctor is UB).
void ensureStarted();

} // namespace TopEngine
