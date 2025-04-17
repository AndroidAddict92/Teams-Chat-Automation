#pragma once
#include <string>
#include <vector>

namespace Url {
    std::string UrlEncode(const std::string& value);
    std::string GenerateChatURL(
        const std::string& csNumber,
        const std::string& siteName,
        const std::string& centerType,
        const std::vector<std::string>& users,
        const std::vector<std::string>& extraEmails
    );
    void OpenURL(const std::string& url);
}