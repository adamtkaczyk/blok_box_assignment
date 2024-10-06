#include "backup_logger.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>

BackupLogger::BackupLogger() {
}

void BackupLogger::InitLoggers(const std::vector<std::string>& symbols) {
    for (const auto& symbol : symbols) {
        InitLoggerForSymbol(symbol);
    }
}

void BackupLogger::InitLoggerForSymbol(const std::string& symbol) {
    if (loggers_.find(symbol) != loggers_.end()) {
        return;
    }

    auto filename = symbol + ".log";

    auto logger = spdlog::basic_logger_mt<spdlog::async_factory>(symbol, filename);
    
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] %v");
    logger->flush_on(spdlog::level::info);

    loggers_[symbol] = logger;
}

void BackupLogger::LogInfo(const std::string& symbol, const std::string& message) {
    if (loggers_.find(symbol) == loggers_.end()) {
        InitLoggerForSymbol(symbol);
    }

    loggers_.at(symbol)->info(message);
}