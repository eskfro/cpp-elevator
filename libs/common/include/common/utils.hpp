#pragma once

#include "common/types.hpp"
#include <string_view>

namespace ansi {
    inline constexpr std::string_view green = "\033[32m";
    inline constexpr std::string_view yellow = "\033[33m";
    inline constexpr std::string_view red = "\033[31m";
    inline constexpr std::string_view reset = "\033[0m";
} // namespace ansi

namespace elev::common {

inline std::string BtnTypeToString(BtnType btn) {
    if (btn == BtnType::Cab) return "Cab";
    if (btn == BtnType::HallUp) return "Hall up";
    if (btn == BtnType::HallDown) return "Hall down";
    return "Error";
}

inline void PrintBtnPress(int elevID, int floor, BtnType btn) {
    std::cout << "[HW] e-" << elevID << " : " << "Button (" <<
    floor << ", "<< BtnTypeToString(btn) << ") pressed" << "\n"; 
}

inline void Print(std::string_view s) { std::cout << s << "\n"; }

inline void PrintError(std::string_view msg) {
    std::cerr << ansi::red << msg << ansi::reset <<"\n";
}

inline void PrintWarning(std::string_view msg) {
    std::cout << ansi::yellow << msg << ansi::reset << "\n";
}

inline void Abort(std::string_view s) {
    PrintError(s);
    abort();
}

inline void PrintFSM(std::string_view event_msg, int elev_id) {
    std::cout << ansi::green << "[FSM] e-" << elev_id << " : " << event_msg << ansi::reset << "\n";
}

} // namespace elev::common