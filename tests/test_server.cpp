// tests/test_server.cpp

#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include "../inc/Server.hpp"
#include "../inc/Location.hpp"
#include "../inc/Utils.hpp"

// ------------------------------------------------------------------------------------------------
// (1) ConfigParser stub: "default.conf"를 최소한 수준으로 파싱해서 Server/Location에 채워넣는다.
// ------------------------------------------------------------------------------------------------
class ConfigParser {
public:
    ConfigParser(const std::string& path)
        : _path(path)
    {
    }

    std::vector<Server> parse() {
        std::vector<Server> result;
        std::ifstream ifs(_path.c_str());
        assert(ifs.is_open() && "Failed to open config file");

        std::string line;
        bool in_server = false;
        bool in_location = false;
        Server* current_srv = NULL;
        Location* current_loc = NULL;

        // server 블록을 열고 닫는 중첩 레벨을 추적하기 위한 카운터
        int brace_level = 0;

        while (std::getline(ifs, line)) {
            // 1) 주석(# 이후) 제거
            size_t comm_pos = line.find('#');
            if (comm_pos != std::string::npos) {
                line = line.substr(0, comm_pos);
            }

            // 2) 앞뒤 공백 트리밍
            std::string trimmed;
            {
                std::stringstream tmp(line);
                std::string word;
                bool first = true;
                while (tmp >> word) {
                    if (!first) trimmed += " ";
                    trimmed += word;
                    first = false;
                }
            }
            if (trimmed.empty()) continue;

            // 3) 토큰화: 첫 단어 읽기
            std::stringstream ss(trimmed);
            std::string token;
            ss >> token;

            // 4) server { 시작
            if (!in_server && token == "server") {
                in_server = true;
                brace_level = 0;
                // 새로운 Server 객체 생성
                ServerManager* dummy_mgr = NULL;
                Config*       dummy_cfg = NULL;
                result.push_back(Server(dummy_mgr, "", "", dummy_cfg));
                current_srv = &result.back();
                continue;
            }

            if (in_server) {
                // “{” 개수 세기
                if (trimmed.find('{') != std::string::npos) {
                    brace_level += 1;
                    // location 블록이 시작되면
                    if (token == "location") {
                        in_location = true;
                        // location URI까지 읽어둠
                        std::string uri;
                        ss >> uri; // e.g. "/images"
                        // {는 그 뒤에 붙어 있으므로, 따로 처리하지 않아도 됨.
                        current_srv->addLocation(Location());          // 우선 빈 Location 추가
                        current_loc = const_cast<Location*>(&current_srv->getLocations().back());
                        current_loc->setUri(uri);
                    }
                    continue; // 다음 줄로
                }

                // “}” 개수 세기
                if (trimmed.find('}') != std::string::npos) {
                    // location 블록 끝
                    if (in_location) {
                        in_location = false;
                        current_loc = NULL;
                        brace_level -= 1;
                        continue;
                    }
                    // server 블록 끝
                    brace_level -= 1;
                    if (brace_level < 0) {
                        in_server = false;
                        current_srv = NULL;
                    }
                    continue;
                }

                // 5) server 블록 내부의 지시문 처리
                if (!in_location) {
                    // server 레벨 지시문
                    if (token == "listen") {
                        // 형식: listen 127.0.0.1:8080;
                        std::string addr_port;
                        ss >> addr_port;
                        // “:8080;”에서 “;” 제거
                        if (!addr_port.empty() && addr_port.back() == ';')
                            addr_port.pop_back();
                        // “127.0.0.1:8080”을 host, port로 분리
                        size_t colon = addr_port.find(':');
                        if (colon != std::string::npos) {
                            std::string host = addr_port.substr(0, colon);
                            int port = std::atoi(addr_port.substr(colon + 1).c_str());
                            current_srv->setHost(host);
                            current_srv->setPort(port);
                        }
                    }
                    else if (token == "server_name") {
                        // 형식: server_name example.com www.example.com;
                        std::string name;
                        while (ss >> name) {
                            if (!name.empty() && name.back() == ';')
                                name.pop_back();
                            current_srv->addServerName(name);
                        }
                    }
                    else if (token == "root") {
                        // 형식: root /var/www/html;
                        std::string path;
                        ss >> path;
                        if (!path.empty() && path.back() == ';')
                            path.pop_back();
                        current_srv->setRootPath(path);
                    }
                    else if (token == "index") {
                        // 형식: index index.html index.htm;
                        std::vector<std::string> idxs;
                        std::string fname;
                        while (ss >> fname) {
                            // 마지막에 “;”가 붙어 있으면 한 번에 제거
                            if (!fname.empty() && fname.back() == ';') {
                                fname.pop_back();
                                idxs.push_back(fname);
                                break;
                            }
                            idxs.push_back(fname);
                        }
                        current_srv->setIndexFiles(idxs);
                    }
                    else if (token == "autoindex") {
                        // 형식: autoindex on; 또는 autoindex off;
                        std::string opt;
                        ss >> opt;
                        if (!opt.empty() && opt.back() == ';')
                            opt.pop_back();
                        if (opt == "on")
                            current_srv->setAutoindex(true);
                        else
                            current_srv->setAutoindex(false);
                    }
                    else if (token == "upload_store") {
                        // 형식: upload_store /tmp/uploads;
                        std::string path;
                        ss >> path;
                        if (!path.empty() && path.back() == ';')
                            path.pop_back();
                        current_srv->setHasUploadStore(true);
                        current_srv->setUploadStore(path);
                    }
                    else if (token == "error_page") {
                        // 형식: error_page 404 /404.html;
                        int code;
                        std::string page;
                        ss >> code >> page;
                        if (!page.empty() && page.back() == ';')
                            page.pop_back();
                        current_srv->addErrorPage(code, page);
                    }
                    // 필요에 따라 다른 지시문도 여기에 추가 가능
                }
                else {
                    // location 블록 내부 지시문 (current_loc 가 NULL이 아님)
                    if (token == "root") {
                        std::string path;
                        ss >> path;
                        if (!path.empty() && path.back() == ';')
                            path.pop_back();
                        current_loc->setRootPath(path);
                    }
                    else if (token == "index") {
                        std::vector<std::string> idxs;
                        std::string fname;
                        while (ss >> fname) {
                            if (!fname.empty() && fname.back() == ';') {
                                fname.pop_back();
                                idxs.push_back(fname);
                                break;
                            }
                            idxs.push_back(fname);
                        }
                        current_loc->setIndexFiles(idxs);
                    }
                    else if (token == "allow_methods") {
                        std::set<std::string> methods;
                        std::string m;
                        while (ss >> m) {
                            if (!m.empty() && m.back() == ';')
                                m.pop_back();
                            methods.insert(m);
                        }
                        current_loc->setAllowMethods(methods);
                    }
                    else if (token == "cgi_extension") {
                        // 형식: cgi_extension .php /usr/bin/php-cgi;
                        std::string ext, binpath;
                        ss >> ext >> binpath;
                        if (!binpath.empty() && binpath.back() == ';')
                            binpath.pop_back();
                        std::set<std::string> exts = current_loc->getCgiExtensions();
                        exts.insert(ext);
                        current_loc->setCgiExtensions(exts);
                        // 실제로는 “맵” 형태로 저장해야 하지만, 테스트용으로 Set만 사용
                    }
                    else if (token == "autoindex") {
                        std::string opt;
                        ss >> opt;
                        if (!opt.empty() && opt.back() == ';')
                            opt.pop_back();
                        current_loc->setAutoindex(opt == "on");
                    }
                    else if (token == "return") {
                        // 형식: return 302 http://redirect.local;
                        int code;
                        std::string url;
                        ss >> code >> url;
                        if (!url.empty() && url.back() == ';')
                            url.pop_back();
                        current_loc->setRedirect(code, url);
                    }
                    else if (token == "upload_store") {
                        std::string path;
                        ss >> path;
                        if (!path.empty() && path.back() == ';')
                            path.pop_back();
                        current_loc->setHasUploadStore(true);
                        current_loc->setUploadStore(path);
                    }
                }
            }
        }

        ifs.close();
        return result;
    }

private:
    std::string _path;
};

// ------------------------------------------------------------------------------------------------
// (2) Server/Location 출력 함수: 모든 멤버를 눈으로 확인하기 위해 std::cout 으로 찍어준다.
// ------------------------------------------------------------------------------------------------
static void printLocation(const Location& loc) {
    std::cout << "  [Location]\n";
    std::cout << "    URI: " << loc.getUri() << "\n";
    std::cout << "    RootPath: " << loc.getRootPath() << "\n";

    // AllowMethods
    const std::set<std::string> methods = loc.getAllowMethods();
    std::cout << "    AllowMethods: ";
    if (methods.empty()) {
        std::cout << "(none)";
    } else {
        bool first = true;
        for (std::set<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
            if (!first) std::cout << ", ";
            std::cout << *it;
            first = false;
        }
    }
    std::cout << "\n";

    // IndexFiles
    const std::set<std::string> idxs = loc.getIndexFiles();
    std::cout << "    IndexFiles: ";
    if (idxs.empty()) {
        std::cout << "(none)";
    } else {
        bool first = true;
        for (std::set<std::string>::const_iterator it = idxs.begin(); it != idxs.end(); ++it) {
            if (!first) std::cout << ", ";
            std::cout << *it;
            first = false;
        }
    }
    std::cout << "\n";

    // CgiExtensions
    const std::set<std::string> cgis = loc.getCgiExtensions();
    std::cout << "    CgiExtensions: ";
    if (cgis.empty()) {
        std::cout << "(none)";
    } else {
        bool first = true;
        for (std::set<std::string>::const_iterator it = cgis.begin(); it != cgis.end(); ++it) {
            if (!first) std::cout << ", ";
            std::cout << *it;
            first = false;
        }
    }
    std::cout << "\n";

    std::cout << "    Autoindex: " << (loc.getAutoindex() ? "on" : "off") << "\n";

    std::cout << "    HasRedirect: " << (loc.hasRedirect() ? "yes" : "no");
    if (loc.hasRedirect()) {
        std::cout << " (code=" << loc.getRedirectCode()
                  << ", url=" << loc.getRedirectUrl() << ")";
    }
    std::cout << "\n";

    std::cout << "    HasUploadStore: " << (loc.hasUploadStore() ? "yes" : "no");
    if (loc.hasUploadStore()) {
        std::cout << " (path=" << loc.getUploadStore() << ")";
    }
    std::cout << "\n";
}

static void printServer(const Server& srv) {
    std::cout << "=== Server ===\n";
    std::cout << "Host: " << srv.getHost() << "\n";
    std::cout << "Port: " << srv.getPort() << "\n";

    // ServerNames
    const std::vector<std::string> names = srv.getServerNames();
    std::cout << "ServerNames: ";
    if (names.empty()) {
        std::cout << "(none)";
    } else {
        for (size_t i = 0; i < names.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << names[i];
        }
    }
    std::cout << "\n";

    // RootPath
    std::cout << "RootPath: " << srv.getRootPath() << "\n";

    // IndexFiles (server 레벨)
    const std::vector<std::string> idxs = srv.getIndexFiles();
    std::cout << "IndexFiles: ";
    if (idxs.empty()) {
        std::cout << "(none)";
    } else {
        for (size_t i = 0; i < idxs.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << idxs[i];
        }
    }
    std::cout << "\n";

    // Autoindex
    std::cout << "Autoindex: " << (srv.getAutoindex() ? "on" : "off") << "\n";

    // UploadStore
    std::cout << "HasUploadStore: " << (srv.hasUploadStore() ? "yes" : "no");
    if (srv.hasUploadStore()) {
        std::cout << " (path=" << srv.getUploadStore() << ")";
    }
    std::cout << "\n";

    // DefaultErrorPage (404 등)
    std::cout << "DefaultErrorPage: " << srv.getDefaultErrorPage() << "\n";

    // ErrorPages 맵 전체 출력
    // 현재 private 멤버라 직접 접근 불가능하므로, getter를 추가하거나, 
    // 테스트를 위한 임시 접근자(getErrorPages())를 만들어도 됩니다.
    {
        std::map<int, std::string> temp; 
        // 예: 만약 Server 클래스에 getErrorPages()가 없다면, 
        // test_server.cpp 상단에 “친구 선언” 해도 되고, 임시 getter를 추가해 주세요.
        // temp = srv.getErrorPages(); 
        // 여기서는 예시로 “404”만 출력하도록 하겠습니다.
        std::cout << "ErrorPages:\n";
        // (실제 코드에서는 getErrorPages()를 호출하세요)
        temp = srv.getErrorPages(); 
        if (temp.empty()) {
            std::cout << "  (none)\n";
        } else {
            for (std::map<int, std::string>::const_iterator it = temp.begin(); it != temp.end(); ++it) {
                std::cout << "  [" << it->first << "] -> " << it->second << "\n";
            }
        }
    }

    // Locations
    const std::vector<Location>& locs = srv.getLocations();
    std::cout << "Locations count: " << locs.size() << "\n";
    for (size_t i = 0; i < locs.size(); ++i) {
        printLocation(locs[i]);
    }
    std::cout << "===============\n\n";
}

// ------------------------------------------------------------------------------------------------
// (3) main(): 파서 실행 → 파싱 결과를 모두 출력
// ------------------------------------------------------------------------------------------------
int main() {
    const std::string cfg_path = "tests/fixtures/default.conf";
    ConfigParser parser(cfg_path);
    std::vector<Server> servers = parser.parse();
    assert(!servers.empty() && "default.conf 을 파싱해서 최소 하나의 Server가 생성되어야 합니다");

    // (4) 모든 Server 객체를 순회하며, 멤버값을 출력
    for (size_t i = 0; i < servers.size(); ++i) {
        std::cout << "[Parsed Server #" << i << "]\n";
        printServer(servers[i]);
    }

    // (5) 필요하다면, 여기서 assert로 각 필드를 재검증 가능
    // 예)
    // assert(servers[0].getIndexFiles().size() == 2);
    // assert(servers[0].getIndexFiles()[0] == "index.html");

    return 0;
}
