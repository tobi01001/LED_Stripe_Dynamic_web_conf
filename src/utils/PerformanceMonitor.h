/*************************************************************
 * PerformanceMonitor.h - Performance monitoring and optimization
 * 
 * Provides simple performance monitoring capabilities to help
 * identify bottlenecks and optimize the LED controller.
 *************************************************************/

#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <Arduino.h>

#ifdef DEBUG

/**
 * Simple performance monitoring utility for debug builds
 */
class PerformanceMonitor {
public:
    struct Metrics {
        uint32_t loopCount = 0;
        uint32_t maxLoopTime = 0;
        uint32_t totalLoopTime = 0;
        uint32_t lastLoopTime = 0;
        uint32_t freeHeap = 0;
        uint32_t minFreeHeap = UINT32_MAX;
        float avgLoopTime = 0.0f;
        uint32_t lastUpdate = 0;
    };

    static void beginLoop();
    static void endLoop();
    static void updateMemoryStats();
    static const Metrics& getMetrics() { return metrics; }
    static void printMetrics();
    static void reset();

    // Specific timing helpers
    static void startTiming(const char* name);
    static void endTiming(const char* name);

private:
    static Metrics metrics;
    static uint32_t loopStartTime;
    static uint32_t timingStartTime;
    static const char* currentTimingName;
    
    static constexpr uint32_t METRICS_UPDATE_INTERVAL = 5000; // 5 seconds
};

// Convenience macros
#define PERF_BEGIN_LOOP() PerformanceMonitor::beginLoop()
#define PERF_END_LOOP() PerformanceMonitor::endLoop()
#define PERF_UPDATE_MEMORY() PerformanceMonitor::updateMemoryStats()
#define PERF_START_TIMING(name) PerformanceMonitor::startTiming(name)
#define PERF_END_TIMING(name) PerformanceMonitor::endTiming(name)

#else

// No-op macros for release builds
#define PERF_BEGIN_LOOP()
#define PERF_END_LOOP()
#define PERF_UPDATE_MEMORY()
#define PERF_START_TIMING(name)
#define PERF_END_TIMING(name)

#endif // DEBUG

#endif // PERFORMANCE_MONITOR_H