#pragma once 

#include <unordered_map>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

class BackupLogger {
public:
    BackupLogger();
    void InitLoggers(const std::vector<std::string>& symbols);
    void LogInfo(const std::string& symbol, const std::string& message);

private:
    void InitLoggerForSymbol(const std::string& symbol);

    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
};
