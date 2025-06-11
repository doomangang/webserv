// ConfigParser.cpp

#include "../inc/ConfigParser.hpp"

ConfigParser::ConfigParser() {}

ConfigParser::ConfigParser(const ConfigParser& other) {
    *this = other;
}

ConfigParser::~ConfigParser() {}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
    if (this != &other) {
        (void)other;
    }
    return *this;
}

// --- Public parse entry point ---

Config ConfigParser::parseConfigFile(const std::string& path, char* envp[]) {
    std::string raw = readConfigFile(path);

    std::string cleaned = preprocessConfig(raw);

    std::vector<std::string> serverBlocks = extractBlocks(cleaned, "server");

    if (serverBlocks.empty()) {
        throw std::runtime_error("No server block found in config");
    }

    std::vector<Server> servers;
    servers.reserve(serverBlocks.size());
    for (std::vector<std::string>::iterator it = serverBlocks.begin();
         it != serverBlocks.end(); ++it)
    {
        const std::string& serverText = *it;
        Server srv = parseServerBlock(serverText);
        servers.push_back(srv);
    }

    {
        std::set<unsigned short> ports;
        for (size_t i = 0; i < servers.size(); ++i) {
            unsigned short p = servers[i].getPort();
            if (!ports.insert(p).second) {
                std::ostringstream oss; oss << p;
                throw std::runtime_error("Duplicate listen port: " + oss.str());
            }
        }
    }

    return Config(servers, envp);
}

// File reading and preprocessing 

void ConfigParser::loadConfigFile(std::string path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        throw ConfigLoadException("No such file: " + path);

    if (!S_ISREG(sb.st_mode))
        throw ConfigLoadException("Directory or unreadable: " + path);

    if (sb.st_size == 0)
        throw ConfigLoadException("File is empty: " + path);

    std::ifstream ifs(path.c_str());
    if (!ifs.is_open())
        throw ConfigLoadException("Cannot open file: " + path);

    if (path.size() < 5 || path.substr(path.size() - 5) != ".conf")
        throw ConfigLoadException("Wrong file format - should be *.conf: " + path);

    ifs.close();
}

std::string ConfigParser::readConfigFile(std::string path) {
    // Validate path and existence
    loadConfigFile(path);

    // Open binary to avoid CRLF translation
    std::ifstream ifs(path.c_str(), std::ios::in | std::ios::binary);
    if (!ifs.is_open())
        throw ConfigReadException("Cannot open file: " + path);

    ifs.seekg(0, std::ios::end);
    std::streamoff size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::string content;
    content.reserve(static_cast<size_t>(size));

    content.assign(
        std::istreambuf_iterator<char>(ifs),
        std::istreambuf_iterator<char>()
    );
    ifs.close();

    // Strip UTF-8 BOM if present
    if (content.size() >= 3
        && static_cast<unsigned char>(content[0]) == 0xEF
        && static_cast<unsigned char>(content[1]) == 0xBB
        && static_cast<unsigned char>(content[2]) == 0xBF)
    {
        content.erase(0, 3);
    }

    // Normalize line endings: "\r\n" -> "\n", lone '\r' -> '\n'
    {
        size_t pos = 0;
        while ((pos = content.find("\r\n", pos)) != std::string::npos)
            content.replace(pos, 2, "\n");
        pos = 0;
        while ((pos = content.find('\r', pos)) != std::string::npos)
            content[pos] = '\n';
    }

    // Disallow embedded null bytes
    if (content.find('\0') != std::string::npos)
        throw ConfigReadException("File contains null bytes: " + path);

    return content;
}

std::string ConfigParser::preprocessConfig(std::string raw) {
    std::istringstream iss(raw);
    std::string line;
    std::string cleaned;

    while (std::getline(iss, line)) {
        // Remove comments starting with '#'
        size_t pos = line.find('#');
        if (pos != std::string::npos)
            line.erase(pos);

        // Remove comments starting with "//"
        pos = line.find("//");
        if (pos != std::string::npos)
            line.erase(pos);

        HttpUtils::trim(line);
        if (!line.empty()) {
            cleaned += line;
            cleaned += '\n';
        }
    }
    return cleaned;
}

// Block extraction (server or location) 

std::vector<std::string>
ConfigParser::extractBlocks(const std::string text, const std::string blockType)
{
    std::vector<std::string> blocks;
    const std::string keyword = blockType;
    size_t i = 0, N = text.size();

    while (i < N) {
        // Find next occurrence of blockType
        size_t pos = text.find(keyword, i);
        if (pos == std::string::npos)
            break;

        // Find '{' after keyword
        size_t openBrace = text.find('{', pos + keyword.length());
        if (openBrace == std::string::npos)
            throw std::runtime_error("Parse error: Missing '{' after " + blockType);

        // Depth‐count braces to find matching '}'
        int depth = 1;
        size_t j = openBrace + 1;
        while (j < N && depth > 0) {
            if (text[j] == '{') depth++;
            else if (text[j] == '}') depth--;
            j++;
        }
        if (depth != 0)
            throw std::runtime_error("Parse error: Unmatched '{' in " + blockType + " block");

        // Extract full substring from pos to j (including braces)
        blocks.push_back(text.substr(pos, j - pos));
        i = j; // Move index past this block
    }
    return blocks;
}

// --- Server block parsing ---

Server ConfigParser::parseServerBlock(std::string serverText) {
    // 1) Extract the content inside the outer braces
    size_t open  = serverText.find('{');
    size_t close = serverText.rfind('}');
    if (open == std::string::npos || close == std::string::npos || close <= open)
        throw std::runtime_error("Invalid server block braces");

    std::string body = serverText.substr(open + 1, close - open - 1);

    // 2) Extract all nested "location { … }" blocks
    std::vector<std::string> locBlocks = extractBlocks(body, "location");

    // 3) Remove each location block from body to isolate server-level directives
    std::string serverCore = body;
    for (std::vector<std::string>::iterator it = locBlocks.begin();
         it != locBlocks.end(); ++it)
    {
        size_t p = serverCore.find(*it);
        if (p != std::string::npos)
            serverCore.erase(p, it->length());
    }

    // 4) Before splitting on ';', verify each non-blank, non-location line ends with ';'
    {
        std::istringstream iss(serverCore);
        std::string line;
        while (std::getline(iss, line)) {
            HttpUtils::trim(line);
            if (line.empty()) continue;
            // If line begins a nested block (unlikely after removal), skip
            if (line.find("location") == 0 && line.find('{') != std::string::npos)
                continue;
            // Must end with ';'
            if (line[line.size() - 1] != ';')
                throw std::runtime_error("Missing semicolon in server directive: " + line);
        }
    }

    // 5) Split serverCore on ';' and parse each directive
    // 기본값 설정
    Server srv;
    srv.setHost("0.0.0.0");                    // 기본 호스트
    srv.setPort(80);                           // 기본 포트
    srv.setRequestUriLimitSize(8192);          // 8KB
    srv.setRequestHeaderLimitSize(8192);       // 8KB
    srv.setLimitClientBodySize(1048576);       // 1MB
    srv.setAutoindex(false);                   // 기본적으로 비활성화
    
    // 기본 인덱스 파일
    std::vector<std::string> defaultIndex;
    defaultIndex.push_back("index.html");
    defaultIndex.push_back("index.htm");
    srv.setIndexFiles(defaultIndex);
    
    // 기본 에러 페이지
    srv.setDefaultErrorPage("/errors/default.html");

    std::vector<std::string> stmts = HttpUtils::splitBySemicolon(serverCore);
    for (size_t i = 0; i < stmts.size(); ++i) {
        std::string stmt = stmts[i];
        HttpUtils::trim(stmt);
        if (stmt.empty()) continue;

        if (stmt.find("listen ") == 0) {
            // "listen host:port" or "listen port"
            std::string args = stmt.substr(7);
            HttpUtils::trim(args);
            size_t colon = args.find(':');
            if (colon != std::string::npos) {
                srv.setHost(args.substr(0, colon));
                srv.setPort(static_cast<unsigned short>(
                    std::atoi(args.substr(colon + 1).c_str())));
            } else {
                srv.setHost("0.0.0.0");
                srv.setPort(static_cast<unsigned short>(std::atoi(args.c_str())));
            }
        }
        else if (stmt.find("server_name ") == 0) {
            // e.g. "server_name a.com b.com"
            std::vector<std::string> names = HttpUtils::splitWords(stmt.substr(12));
            srv.setServerNames(names);
        }
        else if (stmt.find("root ") == 0) {
            // e.g. "root /var/www"
            std::string path = stmt.substr(5);
            HttpUtils::trim(path);
            srv.setRootPath(path);
        }
        else if (stmt.find("index ") == 0) {
            // e.g. "index index.html index.htm"
            std::vector<std::string> files = HttpUtils::splitWords(stmt.substr(6));
            srv.setIndexFiles(files);
        }
        else if (stmt.find("client_max_body_size ") == 0) {
            // e.g. "client_max_body_size 1000000"
            std::string num = stmt.substr(22);
            HttpUtils::trim(num);
            srv.setLimitClientBodySize(static_cast<size_t>(std::atoi(num.c_str())));
        }
        else if (stmt.find("error_page ") == 0) {
            // e.g. "error_page 404 /404.html"
            std::vector<std::string> parts = HttpUtils::splitWords(stmt.substr(11));
            if (parts.size() >= 2) {
                int code = std::atoi(parts[0].c_str());
                std::string path = parts[1];
                srv.addErrorPage(code, path);
            }
        }
        else if (stmt.find("autoindex ") == 0) {
            // e.g. "autoindex on"/"autoindex off"
            std::string onoff = stmt.substr(10);
            HttpUtils::trim(onoff);
            srv.setAutoindex(onoff == "on");
        }
        else if (stmt.find("upload_store ") == 0) {
            // e.g. "upload_store /tmp/uploads"
            std::string up = stmt.substr(13);
            HttpUtils::trim(up);
            srv.setUploadStore(up);
        }
        // unknown server-level directives are ignored
    }

    // 6) Parse each nested location block and add to server
    for (size_t k = 0; k < locBlocks.size(); ++k) {
        Location loc = parseLocationBlock(locBlocks[k]);
        srv.addLocation(loc);
    }

    validateServerBlock(srv);
    return srv;
}

// --- Location block parsing ---

Location ConfigParser::parseLocationBlock(const std::string& locText) {
    // 1) Extract URI from header "location <uri> {"
    std::string uri = extractLocationUri(locText);
    Location loc;
    loc.setUri(uri);

    // 기본값 설정
    loc.setAutoindex(false);
    
    // 기본 메서드 설정
    std::set<Method> defaultMethods;
    defaultMethods.insert(GET);
    defaultMethods.insert(POST);
    defaultMethods.insert(DELETE);
    loc.setAllowMethods(defaultMethods);
    
    // 2) Extract body inside braces
    std::string body = extractBlockBody(locText);

    // 3) Split body into semicolon-terminated statements
    std::vector<std::string> stmts = splitStatements(body);

    // 4) Parse each statement
    for (size_t i = 0; i < stmts.size(); ++i) {
        std::string stmt = stmts[i];
        HttpUtils::trim(stmt);
        if (stmt.empty()) continue;

        if (stmt.find("methods ") == 0) {
            parseMethodsDirective(loc, stmt);
        }
        else if (stmt.find("return ") == 0) {
            parseReturnDirective(loc, stmt);
        }
        else if (stmt.find("root ") == 0) {
            parseRootDirective(loc, stmt);
        }
        else if (stmt.find("index ") == 0) {
            parseIndexDirective(loc, stmt);
        }
        else if (stmt.find("autoindex ") == 0) {
            parseAutoindexDirective(loc, stmt);
        }
        else if (stmt.find("cgi ") == 0) {
            parseCgiDirective(loc, stmt);
        }
        else if (stmt.find("upload_store ") == 0) {
            parseUploadStoreDirective(loc, stmt);
        }
        // 그 외 지시자는 무시
    }

    // 5) Default methods 설정 (GET, POST, DELETE)
    if (loc.getAllowMethods().empty()) {
        std::set<Method> defaults;
        defaults.insert(GET);
        defaults.insert(POST);
        defaults.insert(DELETE);
        loc.setAllowMethods(defaults);
    }

    return loc;
}

// --- Location directive helpers ---

std::string ConfigParser::extractLocationUri(const std::string& text) {
    size_t open = text.find('{');
    if (open == std::string::npos)
        throw std::runtime_error("Invalid location block: no '{'");
    std::string header = text.substr(0, open);
    HttpUtils::trim(header);
    // header: "location <uri>"
    size_t sp = header.find(' ');
    if (sp == std::string::npos)
        throw std::runtime_error("Invalid location header: " + header);
    std::string uri = header.substr(sp + 1);
    HttpUtils::trim(uri);
    return uri;
}

std::string ConfigParser::extractBlockBody(const std::string& text) {
    size_t open  = text.find('{');
    size_t close = text.rfind('}');
    if (open == std::string::npos || close == std::string::npos || close <= open)
        throw std::runtime_error("Invalid block braces");
    return text.substr(open + 1, close - open - 1);
}

std::vector<std::string> ConfigParser::splitStatements(const std::string& body) {
    return HttpUtils::splitBySemicolon(body);
}

void ConfigParser::parseMethodsDirective(Location& loc, const std::string& stmt) {
    // "methods GET POST DELETE"
    loc.clearAllowMethods();

    std::vector<std::string> words = HttpUtils::splitWords(stmt.substr(8));
    std::set<Method> methods;
    
    for (size_t i = 0; i < words.size(); ++i) {
        const std::string& w = words[i];
        if (w == "GET") methods.insert(GET);
        else if (w == "POST") methods.insert(POST);
        else if (w == "DELETE") methods.insert(DELETE);
        else if (w == "EMPTY") methods.insert(EMPTY);
        else methods.insert(UNKNOWN_METHOD);
    }
    
    loc.setAllowMethods(methods);
}

void ConfigParser::parseReturnDirective(Location& loc, const std::string& stmt) {
    // "return 301 http://..."
    std::vector<std::string> parts = HttpUtils::splitWords(stmt.substr(7));
    if (parts.size() >= 2) {
        int code = std::atoi(parts[0].c_str());
        std::string url = parts[1];
        for (size_t j = 2; j < parts.size(); ++j) {
            url += " ";
            url += parts[j];
        }
        loc.setRedirect(code, url);
    }
}

void ConfigParser::parseRootDirective(Location& loc, const std::string& stmt) {
    // "root /some/path"
    std::string path = stmt.substr(5);
    HttpUtils::trim(path);
    loc.setRootPath(path);
}

void ConfigParser::parseIndexDirective(Location& loc, const std::string& stmt) {
    // "index index.html index.htm"
    std::vector<std::string> files = HttpUtils::splitWords(stmt.substr(6));
    loc.setIndexFiles(files);
}

void ConfigParser::parseAutoindexDirective(Location& loc, const std::string& stmt) {
    // "autoindex on" or "autoindex off"
    std::string onoff = stmt.substr(10);
    HttpUtils::trim(onoff);
    loc.setAutoindex(onoff == "on");
}

void ConfigParser::parseCgiDirective(Location& loc, const std::string& stmt) {
    // "cgi .php .py"
    std::vector<std::string> exts = HttpUtils::splitWords(stmt.substr(4));
    std::set<std::string> s(exts.begin(), exts.end());
    loc.setCgiExtensions(s);
}

void ConfigParser::parseUploadStoreDirective(Location& loc, const std::string& stmt) {
    // "upload_store /tmp/uploads"
    std::string up = stmt.substr(13);
    HttpUtils::trim(up);
    loc.setHasUploadStore(true);
    loc.setUploadStore(up);
}

void ConfigParser::validatePort(int port) {
    if (port < 1 || port > 65535) {
        throw InvalidPortException(port);
    }
}

void ConfigParser::validatePath(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        throw InvalidPathException(path);
    }
}

void ConfigParser::validateBodySize(size_t size) {
    const size_t MAX_BODY_SIZE = 100 * 1024 * 1024; // 100MB
    if (size > MAX_BODY_SIZE) {
        throw std::runtime_error("client_max_body_size exceeds maximum allowed: " + 
                               HttpUtils::toString(MAX_BODY_SIZE));
    }
}

void ConfigParser::validateServerBlock(const Server& srv) {
    // 필수 항목 체크
    if (srv.getPort() == 0) {
        throw MissingDirectiveException("listen");
    }
    validatePort(srv.getPort());

    validateRootConfiguration(srv);
    
    // 각 location 검증
    const std::vector<Location>& locations = srv.getLocations();
    for (size_t i = 0; i < locations.size(); ++i) {
        if (!locations[i].getRootPath().empty()) {
            validatePath(locations[i].getRootPath());
        }
    }

    if (!srv.getRootPath().empty()) {
        validatePath(srv.getRootPath());
    }
}

void ConfigParser::validateRootConfiguration(const Server& srv) {
    bool server_has_root = !srv.getRootPath().empty();
    const std::vector<Location>& locations = srv.getLocations();
    
    // Case 1: Location이 없는 경우 - 서버 레벨에 root 필수
    if (locations.empty()) {
        if (!server_has_root) {
            throw MissingDirectiveException("root (required at server level when no locations defined)");
        }
        return;
    }
    
    // Case 2: Location들이 있는 경우 - 각각이 root를 해결할 수 있는지 확인
    std::vector<std::string> problematic_locations;
    
    for (size_t i = 0; i < locations.size(); ++i) {
        const Location& loc = locations[i];
        bool location_has_root = !loc.getRootPath().empty();
        
        // 이 location이 root 경로를 결정할 수 있는가?
        if (!location_has_root && !server_has_root) {
            problematic_locations.push_back(loc.getUri());
        }
    }
    
    // 문제가 있는 location들 리포트
    if (!problematic_locations.empty()) {
        std::string error_msg = "missing root directive for location(s): ";
        for (size_t i = 0; i < problematic_locations.size(); ++i) {
            if (i > 0) error_msg += ", ";
            error_msg += "'" + problematic_locations[i] + "'";
        }
        error_msg += " (provide server-level root or location-specific root)";
        
        throw MissingDirectiveException(error_msg);
    }
    
    std::cout << GREEN << "Root configuration validated successfully:" << RESET << std::endl;
    if (server_has_root) {
        std::cout << "  Server root: " << srv.getRootPath() << std::endl;
    }
    for (size_t i = 0; i < locations.size(); ++i) {
        const Location& loc = locations[i];
        if (!loc.getRootPath().empty()) {
            std::cout << "  Location " << loc.getUri() << " root: " << loc.getRootPath() << std::endl;
        } else {
            std::cout << "  Location " << loc.getUri() << " root: (inherited from server)" << std::endl;
        }
    }
}