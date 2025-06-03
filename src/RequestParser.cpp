#include "RequestParser.hpp"

// 마지막에 남는 조각도 lines.back()에 들어갈 수 있다
static std::vector<std::string> splitByCRLF(const std::string &raw) {
    std::vector<std::string> lines;
    size_t start = 0;

    while (start < raw.size()) {
        size_t pos = raw.find("\r\n", start);
        if (pos == std::string::npos) {
            lines.push_back(raw.substr(start));
            break;
        }
        lines.push_back(raw.substr(start, pos - start));
        start = pos + 2;
    }
    return lines;
}

bool parseRawRequest(const std::string &raw_request, Request &req) {
    std::vector<std::string> lines = splitByCRLF(raw_request);
    if (lines.empty())
        return false;

    // request line
    {
        std::string start_line = Utils::Trim(lines[0]);
        // METHOD SP URI SP VERSION
        size_t p1 = start_line.find(' ');
        if (p1 == std::string::npos) return false;

        size_t p2 = start_line.find(' ', p1 + 1);
        if (p2 == std::string::npos) return false;

        std::string method_str = start_line.substr(0, p1);
        std::string url_str    = start_line.substr(p1 + 1, p2 - p1 - 1);
        std::string ver_str    = start_line.substr(p2 + 1);

        if (method_str.empty() || url_str.empty() || ver_str.empty())
            return false;

        req.SetMethod(method_str);
        req.SetUrl(url_str);
        req.SetVersion(ver_str);
    }

    // Header block
    size_t idx = 1;
    for (; idx < lines.size(); ++idx) {
        std::string line = lines[idx];
        if (line.empty())
            break;

        if (!req.ParseHeaderLine(line))
            return false;
    }


    ssize_t content_len = 0;
    if (req.HasHeader("content-length")) {
        std::string val = req.GetHeaderValue("content-length");
        content_len = std::atoi(val.c_str());
        if (content_len < 0) content_len = 0;
        req.SetBytesToRead(content_len);
        req.ReserveBody(content_len);  // 미리 reserve 해 두면 효율적
    }

    ssize_t remaining = content_len;
    if (content_len > 0 && idx + 1 < lines.size()) {
        std::string accumulated;
        for (size_t j = idx + 1; j < lines.size(); ++j) {
            accumulated += lines[j];
            if (j + 1 < lines.size()) {
                accumulated += "\r\n";
            }
        }

        if (static_cast<ssize_t>(accumulated.size()) > content_len) {
            accumulated = accumulated.substr(0, content_len);
        }
        req.SetBody(accumulated);
        req.AddBodyPos(static_cast<size_t>(accumulated.size()));
    }

    req.SetStatus(COMPLETE);
    return true;
}