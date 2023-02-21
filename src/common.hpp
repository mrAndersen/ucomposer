#pragma once

#include <aecpp.h>
#include <mutex>

inline std::mutex m_printer = {};

inline void success(const std::string &message) {
    std::lock_guard<std::mutex> l(m_printer);
    std::cout << aec::green << message << aec::reset <<  std::endl;
}

inline void error(const std::string &message) {
    std::lock_guard<std::mutex> l(m_printer);
    std::cout << aec::red << message << aec::reset << std::endl;
}

inline void errorExit(const std::string &message) {
    error(message);
    exit(1);
}
