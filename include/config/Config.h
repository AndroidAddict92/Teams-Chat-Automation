#pragma once
#include <vector>
#include <string>
#include <imgui.h>

namespace Config {
    inline constexpr int    WindowWidth = 330;
    inline constexpr int    WindowHeight = 505;
    inline constexpr const char* WindowTitle = "Teams Chat Automation";

    inline const std::vector<std::string> PredefinedUsers = {
        "user1@domain.com",
        "user2@domain.com",
        "user3@domain.com"
    };
    inline const std::vector<const char*> PredefinedNames = {
        "ExampleName1",
        "ExampleName2",
        "ExampleName3"
    };
    inline const std::vector<const char*> CenterTypes = {
        "AC", "FC", "ATM"
    };

    inline const ImU32 ColorA = ImGui::ColorConvertFloat4ToU32({ 0.054f, 0.255f, 0.388f, 1.0f });
    inline const ImU32 ColorB = ImGui::ColorConvertFloat4ToU32({ 0.110f, 0.073f, 0.251f, 1.0f });
}
