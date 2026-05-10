#pragma once
#include <string>
#include <vector>
#include <cctype>

inline std::string format_pulse_preview(const std::vector<int32_t>& pulses, int max_elements = 50) {
    if (pulses.empty()) {
        return "Empty";
    }
    std::string preview = "";
    int count = 0;
    for (int32_t val : pulses) {
        if (count > 0) preview += ", ";
        preview += std::to_string(val);
        count++;
        if (count >= max_elements) {
            preview += "... (" + std::to_string(pulses.size()) + " total)";
            break;
        }
    }
    return preview;
}

inline std::string format_payload(const std::vector<uint8_t>& data) {
    std::string payload = "";
    for (auto b : data) {
        if (isprint(b)) payload += (char)b;
        else payload += '.';
    }
    return payload;
}
