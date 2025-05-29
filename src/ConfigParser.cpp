#include "../inc/ConfigParser.hpp"

ConfigParser::ConfigParser() {
}

ConfigParser::ConfigParser(const ConfigParser& other) {
    *this = other;
}

ConfigParser::~ConfigParser() {
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
    if (this != &other) {
        // assignment code here
    }
    return *this;
}

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
}

Config ConfigParser::parseConfigFile(const std::string& path, char* envp[]) {
    //read disk -> mem
    std::string raw = readConfigFile(path);
    //tidy up
    std::string cleaned = preprocessConfig(raw);
    //chop server blocks
    std::vector<std::string> serverBlocks = extractBlocks(cleaned, std::string("server"));

    //parse into server instance
    std::vector<Server> servers;
    servers.reserve(serverBlocks.size());
    for (std::vector<std::string>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); ++it) {
        const std::string& serverText = *it;
        Server srv = parseServerBlock(serverText);
        servers.push_back(srv);
    }

    return Config(servers, envp);
}

std::string ConfigParser::readConfigFile(std::string path) {
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

    if (content.size() >= 3
        && static_cast<unsigned char>(content[0]) == 0xEF
        && static_cast<unsigned char>(content[1]) == 0xBB
        && static_cast<unsigned char>(content[2]) == 0xBF
    ) {
        content.erase(0,3);
    }

    {
        size_t pos = 0;
        while ((pos = content.find("\r\n", pos)) != std::string::npos)
            content.replace(pos, 2, "\n");
        pos = 0;
        while ((pos = content.find('\r', pos)) != std::string::npos)
            content[pos] = '\n';
    }

    if (content.find('\0') != std::string::npos)
        throw ConfigReadException("File contains null bytes: " + path);

    return content;
}

std::string ConfigParser::preprocessConfig(std::string raw) {
    std::istringstream iss(raw);
    std::string line;
    std::string cleaned;

    while (std::getline(iss, line)) {
        size_t pos = line.find('#');
        if (pos != std::string::npos)
            line.erase(pos);
        
        pos = line.find("//");
        if (pos != std::string::npos)
            line.erase(pos);
        
        trim(line);
        if (!line.empty()) {
            cleaned += line;
            cleaned += '\n';
        }
    }
    return cleaned;
}

std::vector<std::string>    ConfigParser::extractBlocks(const std::string text, const std::string blockType) {
    std::vector<std::string> blocks;
    const std::string keyword = blockType;
    size_t i = 0, N = text.size();

    while (i < N) {
        size_t pos = text.find(keyword, i);
        if (pos == std::string::npos)
            break;

        size_t openBrace = text.find('{', pos + keyword.length());
        if (openBrace == std::string::npos)
            throw std::runtime_error("Parse error: Missing '{' after " + blockType);

        int depth = 1;
        size_t j = openBrace + 1;
        while (j < N && depth > 0) {
            if (text[j] == '{') depth++;
            else if (text[j] == '}') depth--;
            j++;
        }
        if (depth != 0)
            throw std::runtime_error("Parse error: Unmatched '{' in " + blockType + " block");

        blocks.push_back(text.substr(pos, j - pos));
        i = j;
    }
    return blocks;
}

Server  ConfigParser::parseServerBlock(std::string serverText) {
    size_t open = serverText.find('{');
    size_t close = serverText.rfind('}');
    std::string body = serverText.substr(open + 1, close-open-1);

    std::vector<std::string> locBlocks = extractBlocks(body, "location");

    std::string serverCore = body;
    for (std::vector<std::string>::iterator it = locBlocks.begin(); it != locBlocks.end(); ++it) {
        size_t p = serverCore.find(*it);
        if (p != std::string::npos)
            serverCore.erase(p, it->length());
    }

    Server srv;

    std::vector<std::string> stmts = splitBySemicolon(serverCore);
    for (size_t i = 0; i < stmts.size(); ++i) {
        std::string stmt = stmts[i]; trim(stmt);
        if (stmt.find("listen ") == 0) {
            std::string args = stmt.substr(7); trim(args);
            size_t colon = args.find(':');
            if (colon != std::string::npos) {
                srv.setHost(args.substr(0,colon));
                srv.setPort((unsigned short)(std::atoi(args.substr(colon+1).c_str())));
            } else {
                srv.setHost("0.0.0.0");
                srv.setPort((unsigned short)(std::atoi(args.c_str())));
            }
        }
        else if (stmt.find("server_name ") == 0) {
            std::vector<std::string> names = splitWords(stmt.substr(12));
            srv.setServerNames(names);
        }
        else if (stmt.find("client_max_body_size ") == 0) {
            std::string num = stmt.substr(22); trim(num);
            srv.setLimitClientBodySize((size_t)std::atoi(num.c_str()));
        }
        else if (stmt.find("error_page ") == 0) {
            std::vector<std::string> parts = splitWords(stmt.substr(11));
            int code = std::atoi(parts[0].c_str());
            srv.addErrorPage(code, parts[1]);
        }
    }

    // Parse nested locations
    for (size_t k = 0; k < locBlocks.size(); ++k) {
        Location loc = parseLocationBlock(locBlocks[k]);
        srv.addLocation(loc);
    }

    return srv;
}

Location ConfigParser::parseLocationBlock(const std::string& locText) {
    // Extract header "location <uri> {"
    size_t pos = locText.find(' ');
    size_t open = locText.find('{', pos);
    std::string uri = locText.substr(pos+1, open-(pos+1)); trim(uri);

    // Extract body
    size_t close = locText.rfind('}');
    std::string body = locText.substr(open+1, close-open-1);

    Location loc;
    loc.setUri(uri);

    // Parse directives
    std::vector<std::string> stmts = splitBySemicolon(body);
    for (size_t i = 0; i < stmts.size(); ++i) {
        std::string stmt = stmts[i]; trim(stmt);
        if (stmt.find("methods ") == 0) {
            std::vector<std::string> m = splitWords(stmt.substr(8));
            loc.setMethods(m);
        }
        else if (stmt.find("return ") == 0) {
            std::vector<std::string> parts = splitWords(stmt.substr(7));
            loc.setRedirect(std::atoi(parts[0].c_str()), parts[1]);
        }
        else if (stmt.find("root ") == 0) {
            std::string path = stmt.substr(5); trim(path);
            loc.setRootPath(path, true);
        }
        else if (stmt.find("index ") == 0) {
            std::vector<std::string> idx = splitWords(stmt.substr(6));
            loc.setIndexFiles(idx);
        }
        else if (stmt.find("autoindex ") == 0) {
            std::string val = stmt.substr(10); trim(val);
            loc.setAutoIndex(val == "on");
        }
        else if (stmt.find("cgi ") == 0) {
            std::vector<std::string> exts = splitWords(stmt.substr(4));
            loc.setCgiExtensions(exts);
        }
        else if (stmt.find("upload_store ") == 0) {
            std::string path = stmt.substr(13); trim(path);
            loc.setUploadStore(path);
        }
    }

    return loc;
}

void ConfigParser::trim(std::string& s) {
    const char* ws = " \t\n\r";

    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) {
        s.clear();
        return;
    }
    s.erase(0, start);

    size_t end = s.find_last_not_of(ws);
    s.erase(end + 1);
}