#include <url/Url.h>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <cstdlib>

namespace Url {

    std::string UrlEncode(const std::string& value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;
        for (unsigned char c : value) {
            if (isalnum(c) || std::string("-_.~").find(c) != std::string::npos)
                escaped << c;
            else
                escaped << '%' << std::setw(2) << int(c);
        }
        return escaped.str();
    }

    std::string GenerateChatURL(
        const std::string& csNumber,
        const std::string& siteName,
        const std::string& centerType,
        const std::vector<std::string>& recipients
    ) {
        std::string url = "https://teams.microsoft.com/l/chat/0/0?users=";
        // append each recipient with comma
        for (size_t i = 0; i < recipients.size(); ++i) {
            url += UrlEncode(recipients[i]);
            if (i + 1 < recipients.size())
                url += ',';
        }
        // build and append the topic name
        std::string topic = "GROUPCHATTITLE " + csNumber + " " + centerType + " - " + siteName;
        url += "&topicName=" + UrlEncode(topic);
        return url;
    }

    void OpenURL(const std::string& url) {
    #ifdef _WIN32
        std::string cmd = "start \"\" \"" + url + "\"";
        std::system(cmd.c_str());
    #endif
    }

}