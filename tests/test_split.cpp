// tests/test_split.cpp
#include "../inc/ServerManager.hpp"

#include <cassert>
#include <fstream>
#include <sstream>

int main() {
    // 1) default.conf 읽기
    std::ifstream in("tests/fixtures/default.conf");
    std::stringstream ss;
    ss << in.rdbuf();
    std::string raw = ss.str();

    // 2) splitConfigString 호출
    ServerManager mgr;
    std::string global;
    std::vector<std::string> servers;
    bool ok = mgr.splitConfigString(raw, global, servers);

    // 아직 구현 전이니 false가 리턴돼야 테스트 통과
    assert(ok == false);

    // TODO: 나중에 true로 바꾸고 servers.size()==1 등 검증
    return 0;
}
