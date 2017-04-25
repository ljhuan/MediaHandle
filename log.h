#ifndef LOG_H_
#define LOG_H_
#include <string>
#include <iostream>
#include <list>
#include <iomanip>
#include <fstream>
#include "glog/logging.h"
#include "boost/filesystem.hpp"
#include "singleton.h"

namespace Common {
    static std::list<google::LogSink *> TheLogSinkList;
    static const char *const LogSeverityNames[] = { "INFO", "WARNING", "ERROR", "FATAL" };

    class LogSinkDefault : public google::LogSink {
    public:
        LogSinkDefault() {}

        virtual ~LogSinkDefault() {}

        virtual void send(google::LogSeverity severity, const char* full_filename,
            const char* base_filename, int line,
            const struct ::tm* tm_time,
            const char* message, size_t message_len) {
            std::string m = ToString(severity, base_filename, line, tm_time, message, message_len);
            std::cout << m << std::endl;

        }
        virtual void WaitTillSent() {}
        static std::string ToString(google::LogSeverity severity, const char* file, int line,
            const struct ::tm* tm_time,
            const char* message, size_t message_len) {
            std::ostringstream stream;
            static uint32_t processId = 0;
            uint32_t threadId = 0;

            stream << std::setfill('0');
            stream << std::setw(4) << 1900 + tm_time->tm_year << "-"
                << std::setw(2) << 1 + tm_time->tm_mon << "-"
                << std::setw(2) << tm_time->tm_mday
                << ' '
                << std::setw(2) << tm_time->tm_hour << ':'
                << std::setw(2) << tm_time->tm_min << ':'
                << std::setw(2) << tm_time->tm_sec
                << ' '
                << "[" << LogSeverityNames[severity] << "]"
                << ' '
                << "[" << file << ':' << line << "]"
                << ' '
                << std::string(message, message_len);
            return stream.str();
        }
    };

    class LogSinkFileLog : public google::LogSink {
    private:
        std::string svcname_;
        std::string logdir_;
        std::string logfile_;
        std::ofstream logstream_;
        size_t logsize_;

    public:
        explicit LogSinkFileLog(const std::string &svcname) :svcname_(svcname), logdir_("./"),
            logfile_(logdir_ + "/" + svcname + "00.log"), logsize_(0) {
        }

        virtual ~LogSinkFileLog() {
            if (logstream_.is_open()) {
                logstream_.close();
            }
        }

        virtual void send(google::LogSeverity severity, const char* full_filename,
            const char* base_filename, int line,
            const struct ::tm* tm_time,
            const char* message, size_t message_len) {
            if (!logstream_.is_open()) {
                // open file
                logstream_.open(logfile_.c_str(), std::ios_base::app);

                if (boost::filesystem::exists(logfile_)) {
                    boost::filesystem::path cpath(logfile_);
                    logsize_ = (size_t)boost::filesystem::file_size(cpath);
                }
            }

            if (logstream_.is_open()) {
                // check size, 2M
                if (logsize_ > 2048000) {
                    // close
                    logstream_.close();

                    // log rotate
                    for (int i = 8; i >= 0; --i) {
                        std::stringstream ssFrom;
                        std::stringstream ssTo;

                        ssFrom << logdir_ << "/" << svcname_;
                        ssTo << logdir_ << "/" << svcname_;
                        ssFrom << std::setfill('0') << std::setw(2) << i << ".log";
                        ssTo << std::setfill('0') << std::setw(2) << i + 1 << ".log";
                        boost::filesystem::path from(ssFrom.str());
                        boost::filesystem::path to(ssTo.str());
                        boost::system::error_code errorcode;
                        boost::filesystem::copy_file(from, to, errorcode);
                        boost::filesystem::remove(from, errorcode);
                    }

                    // resend
                    send(severity, full_filename, base_filename, line, tm_time, message, message_len);
                } else {
                    std::string m = ToString(severity, base_filename, line, tm_time, message, message_len);
                    logstream_ << m << std::endl;
                    logsize_ += m.length();
                }
            }
        }
        virtual void WaitTillSent() {}
        static std::string ToString(google::LogSeverity severity, const char* file, int line,
            const struct ::tm* tm_time,
            const char* message, size_t message_len) {
            return LogSinkDefault::ToString(severity, file, line, tm_time, message, message_len);
        }
    };

    class MediaLog {
    public:
        MediaLog(const std::string & argv0) {
            try {
                google::InitGoogleLogging(argv0.c_str());
                google::SetLogDestination(google::GLOG_INFO, "");
                google::SetLogDestination(google::GLOG_WARNING, "");
                google::SetLogDestination(google::GLOG_ERROR, "");
                google::SetLogDestination(google::GLOG_FATAL, "");

                google::SetStderrLogging(google::GLOG_FATAL);

                TheLogSinkList.push_back(new LogSinkDefault());
                TheLogSinkList.push_back(new LogSinkFileLog(argv0));

                for (auto ls : TheLogSinkList) {
                    google::AddLogSink(ls);
                }
            } catch (const std::exception& ex) {
                std::cout << "log error:" << ex.what();
            }
        }
        ~MediaLog() {
            for (auto ls : TheLogSinkList) {
                google::RemoveLogSink(ls);
                delete ls;
            }
            google::ShutdownGoogleLogging();
        }

    private:
        SINGLETON_DECL(Common::MediaLog);

    };
}

#endif  // LOG_H
