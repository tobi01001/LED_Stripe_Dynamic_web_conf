/*************************************************************
 * PerformanceMonitor.cpp - Performance monitoring implementation
 *************************************************************/

#include "PerformanceMonitor.h"

#ifdef DEBUG

// Static member definitions
PerformanceMonitor::Metrics PerformanceMonitor::metrics;
uint32_t PerformanceMonitor::loopStartTime = 0;
uint32_t PerformanceMonitor::timingStartTime = 0;
const char* PerformanceMonitor::currentTimingName = nullptr;

void PerformanceMonitor::beginLoop() {
    loopStartTime = micros();
}

void PerformanceMonitor::endLoop() {
    uint32_t loopTime = micros() - loopStartTime;
    
    metrics.loopCount++;
    metrics.lastLoopTime = loopTime;
    metrics.totalLoopTime += loopTime;
    
    if (loopTime > metrics.maxLoopTime) {
        metrics.maxLoopTime = loopTime;
    }
    
    metrics.avgLoopTime = (float)metrics.totalLoopTime / metrics.loopCount;
    
    // Update metrics periodically
    uint32_t now = millis();
    if (now - metrics.lastUpdate > METRICS_UPDATE_INTERVAL) {
        updateMemoryStats();
        printMetrics();
        metrics.lastUpdate = now;
    }
}

void PerformanceMonitor::updateMemoryStats() {
    uint32_t heap = ESP.getFreeHeap();
    metrics.freeHeap = heap;
    
    if (heap < metrics.minFreeHeap) {
        metrics.minFreeHeap = heap;
    }
}

void PerformanceMonitor::printMetrics() {
    Serial.println(F("=== Performance Metrics ==="));
    Serial.print(F("Loop Count: ")); Serial.println(metrics.loopCount);
    Serial.print(F("Avg Loop Time: ")); Serial.print(metrics.avgLoopTime); Serial.println(F(" μs"));
    Serial.print(F("Max Loop Time: ")); Serial.print(metrics.maxLoopTime); Serial.println(F(" μs"));
    Serial.print(F("Last Loop Time: ")); Serial.print(metrics.lastLoopTime); Serial.println(F(" μs"));
    Serial.print(F("Free Heap: ")); Serial.print(metrics.freeHeap); Serial.println(F(" bytes"));
    Serial.print(F("Min Free Heap: ")); Serial.print(metrics.minFreeHeap); Serial.println(F(" bytes"));
    Serial.print(F("Loop Frequency: ")); 
    Serial.print(1000000.0f / metrics.avgLoopTime); Serial.println(F(" Hz"));
    Serial.println(F("========================="));
}

void PerformanceMonitor::reset() {
    metrics = Metrics();
    loopStartTime = 0;
    timingStartTime = 0;
    currentTimingName = nullptr;
}

void PerformanceMonitor::startTiming(const char* name) {
    currentTimingName = name;
    timingStartTime = micros();
}

void PerformanceMonitor::endTiming(const char* name) {
    uint32_t elapsed = micros() - timingStartTime;
    
    if (currentTimingName && strcmp(currentTimingName, name) == 0) {
        Serial.print(F("Timing ["));
        Serial.print(name);
        Serial.print(F("]: "));
        Serial.print(elapsed);
        Serial.println(F(" μs"));
    }
    
    currentTimingName = nullptr;
}

#endif // DEBUG