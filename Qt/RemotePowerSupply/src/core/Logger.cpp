#include "Logger.h"
#include <QCoreApplication>
#include <QFile>


#include <iostream>
#include <mutex>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/async_logger.h>
#include <QFileInfo>

spdlog::logger *logger;
QPlainTextEdit *loggerContentGui;

namespace spdlog
{
    namespace sinks
    {
        template<typename Mutex>
        class qplaintextedit_sink : public spdlog::sinks::base_sink<Mutex>
        {
        private:
            QPlainTextEdit *plainTextEdit;
        public:
            explicit qplaintextedit_sink(QPlainTextEdit *plainTextEdit_)
            {
                this->plainTextEdit = plainTextEdit_;
            }

        protected:
            void sink_it_(const spdlog::details::log_msg &msg) override
            {
                spdlog::memory_buf_t formatted;
                spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

                this->plainTextEdit->moveCursor(QTextCursor::End);
                this->plainTextEdit->insertPlainText( QString( fmt::to_string(formatted).data() ));
            }

            void flush_() override
            {
            }
        };
        using qplaintextedit_sink_mt = qplaintextedit_sink<std::mutex>;
    } // namespace sinks
} // namespace spdlog

void QtCustomOutputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
#ifdef QT_DEBUG
    QFileInfo f(QFile(context.file));
    QString ctx_str = "[" + QString(f.fileName()) + ":" + QString::number(context.line) + "] ";
#else
    QString ctx_str = "";
#endif
    switch (type)
    {
        case QtDebugMsg:
            logger->debug( QString(ctx_str + msg).toStdString());
            break;
        case QtWarningMsg:
            logger->warn( QString(ctx_str + msg).toStdString());
            break;
        case QtCriticalMsg:
            logger->critical( QString(ctx_str + msg).toStdString());
            break;
        case QtFatalMsg:
            logger->error( QString(ctx_str + msg).toStdString());
            break;
        default:
            logger->info( QString(ctx_str + msg).toStdString());
    }
}

void setup_logger()
{
    // Init GUI logs display
    loggerContentGui = new QPlainTextEdit();

    try
    {
        // Init logger components
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdout_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%^%l%$] %v");

        auto gui_qplaintextedit = std::make_shared<spdlog::sinks::qplaintextedit_sink_mt>(loggerContentGui);
        gui_qplaintextedit->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%^%l%$] %v");

        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(QCoreApplication::applicationDirPath().toStdString() + "/logs.log", 1024 * 1024 * 20, 10);
        rotating_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [pid %P] [thread %t] [%^%l%$] %v");

        // Init logger instance
        logger = new spdlog::logger("main", {stdout_sink, rotating_sink, gui_qplaintextedit});
        logger->set_level(spdlog::level::trace);

        // Init global logger
        //spdlog::register_logger(std::shared_ptr<spdlog::logger>(logger));
        spdlog::set_default_logger(std::shared_ptr<spdlog::logger>(logger));
    }
    catch( const spdlog::spdlog_ex &ex )
    {
        std::cout << "Logger initialization failed: " << ex.what() << std::endl;
    }

    // Bind Qt logging functions to spdlog
    qInstallMessageHandler(QtCustomOutputMessage);
}

