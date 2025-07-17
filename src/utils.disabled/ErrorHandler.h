/*************************************************************
 * ErrorHandler.h - Centralized error handling and logging
 * 
 * Provides consistent error handling patterns throughout the
 * refactored LED controller application.
 *************************************************************/

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <Arduino.h>

/**
 * Centralized error handling and logging system
 */
class ErrorHandler {
public:
    enum ErrorLevel {
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    // Static methods for easy access
    static void logError(ErrorLevel level, const String& module, const String& message);
    static void logError(ErrorLevel level, const __FlashStringHelper* module, const __FlashStringHelper* message);
    
    // Helper macros for common logging
    static void info(const String& module, const String& message) { 
        logError(INFO, module, message); 
    }
    static void warning(const String& module, const String& message) { 
        logError(WARNING, module, message); 
    }
    static void error(const String& module, const String& message) { 
        logError(ERROR, module, message); 
    }
    static void critical(const String& module, const String& message) { 
        logError(CRITICAL, module, message); 
    }

    // Flash string versions
    static void info(const __FlashStringHelper* module, const __FlashStringHelper* message) { 
        logError(INFO, module, message); 
    }
    static void warning(const __FlashStringHelper* module, const __FlashStringHelper* message) { 
        logError(WARNING, module, message); 
    }
    static void error(const __FlashStringHelper* module, const __FlashStringHelper* message) { 
        logError(ERROR, module, message); 
    }
    static void critical(const __FlashStringHelper* module, const __FlashStringHelper* message) { 
        logError(CRITICAL, module, message); 
    }

    // System state checking
    static bool checkMemory(const String& context = "");
    static bool checkFileSystem(const String& context = "");
    static void handleCriticalError(const String& reason);

private:
    static const __FlashStringHelper* getLevelString(ErrorLevel level);
    static void writeToSerial(ErrorLevel level, const String& module, const String& message);
    static void writeToFile(ErrorLevel level, const String& module, const String& message);
};

// Convenience macros for cleaner code
#ifdef DEBUG
#define LOG_INFO(module, msg) ErrorHandler::info(F(module), F(msg))
#define LOG_WARNING(module, msg) ErrorHandler::warning(F(module), F(msg))
#define LOG_ERROR(module, msg) ErrorHandler::error(F(module), F(msg))
#define LOG_CRITICAL(module, msg) ErrorHandler::critical(F(module), F(msg))
#else
#define LOG_INFO(module, msg) // No-op in release builds
#define LOG_WARNING(module, msg) ErrorHandler::warning(F(module), F(msg))
#define LOG_ERROR(module, msg) ErrorHandler::error(F(module), F(msg))
#define LOG_CRITICAL(module, msg) ErrorHandler::critical(F(module), F(msg))
#endif

#endif // ERROR_HANDLER_H