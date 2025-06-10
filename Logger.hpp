#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

class Logger {
public:
    enum class Mode { Terminal, File, Both };
    enum class Level { Debug, Info, Warn, Error };

    explicit Logger(Mode mode = Mode::Terminal,
                    const std::string& file_path = "log.txt")
        : mode_(mode), file_path_(file_path) {
        if (mode_ == Mode::File || mode_ == Mode::Both) {
            file_.open(file_path_, std::ios::app);
        }
    }

    ~Logger() {
        if (file_.is_open()) file_.close();
    }

    void log(Level level, const std::string& message) {
        std::string line = timestamp() + " [" + levelToString(level) + "] " + message;
        if (mode_ == Mode::Terminal || mode_ == Mode::Both) {
            std::cout << line << std::endl;
        }
        if ((mode_ == Mode::File || mode_ == Mode::Both) && file_.is_open()) {
            file_ << line << std::endl;
        }
    }

    void log(const std::string& message) { log(Level::Info, message); }

    void setMode(Mode mode) {
        if (mode == mode_) return;
        if ((mode_ == Mode::File || mode_ == Mode::Both) && file_.is_open()) {
            file_.close();
        }
        mode_ = mode;
        if ((mode_ == Mode::File || mode_ == Mode::Both)) {
            if (!file_.is_open()) {
                file_.open(file_path_, std::ios::app);
            }
        }
    }

    void setFilePath(const std::string& path) {
        if (file_.is_open()) file_.close();
        file_path_ = path;
        if (mode_ == Mode::File || mode_ == Mode::Both) {
            file_.open(file_path_, std::ios::app);
        }
    }

private:
    Mode mode_;
    std::string file_path_;
    std::ofstream file_;

    static std::string levelToString(Level level) {
        switch (level) {
        case Level::Debug: return "DEBUG";
        case Level::Info: return "INFO";
        case Level::Warn: return "WARN";
        case Level::Error: return "ERROR";
        }
        return "INFO";
    }

    static std::string timestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buf);
    }
};

#endif // LOGGER_HPP
