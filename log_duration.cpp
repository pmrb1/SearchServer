#include "log_duration.h"

LogDuration::LogDuration(std::string func_name, std::ostream& stream) : func_name_(func_name), stream_(stream) {}

LogDuration::~LogDuration() {
    using namespace std::chrono;
    using namespace std::literals;

    const auto end_time = Clock::now();
    const auto duration = end_time - start_time_;

    func_name_.empty() ? stream_ << "Operation time: "s : stream_ << func_name_ << ": "s;

    stream_ << duration_cast<milliseconds>(duration).count() << " ms"s << std::endl;
}
