#!/bin/bash

echo "======================================"
echo "🚀 WEBSERV 전체 기능 테스트 스위트"
echo "======================================"
echo ""

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 헬퍼 함수들
print_test_header() {
    echo -e "\n${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}🧪 $1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
}

wait_for_server() {
    local port=$1
    local max_wait=10
    local count=0
    
    while [ $count -lt $max_wait ]; do
        if nc -z 127.0.0.1 $port 2>/dev/null; then
            return 0
        fi
        sleep 1
        ((count++))
    done
    return 1
}

cleanup_servers() {
    echo -e "\n${YELLOW}🧹 서버 프로세스 정리 중...${NC}"
    pkill -f "./webserv" 2>/dev/null || true
    sleep 2
}

run_test_suite() {
    local test_name="$1"
    local config_file="$2"
    local test_script="$3"
    local port="$4"
    
    print_test_header "$test_name"
    
    echo "🔧 설정 파일: $config_file"
    echo "📋 테스트 스크립트: $test_script"
    echo "🌐 포트: $port"
    echo ""
    
    # 서버 시작
    echo "🚀 서버 시작 중..."
    timeout 60 ./webserv "$config_file" &
    local server_pid=$!
    
    # 서버 준비 대기
    if wait_for_server $port; then
        echo -e "${GREEN}✅ 서버 시작됨 (PID: $server_pid)${NC}"
        
        # 테스트 실행
        echo "🧪 테스트 실행 중..."
        if ./"$test_script"; then
            echo -e "${GREEN}✅ $test_name 테스트 성공${NC}"
        else
            echo -e "${RED}❌ $test_name 테스트 실패${NC}"
        fi
    else
        echo -e "${RED}❌ 서버 시작 실패${NC}"
    fi
    
    # 서버 종료
    kill $server_pid 2>/dev/null || true
    sleep 2
    
    echo -e "${YELLOW}📋 $test_name 테스트 완료${NC}"
}

# 메인 실행
main() {
    echo "시작 시간: $(date)"
    echo ""
    
    # 기존 서버 프로세스 정리
    cleanup_servers
    
    # webserv 실행 파일 확인
    if [ ! -f "./webserv" ]; then
        echo -e "${RED}❌ ./webserv 실행 파일이 없습니다. 먼저 컴파일하세요.${NC}"
        echo "   make && make clean"
        exit 1
    fi
    
    # 필요한 스크립트들 실행 권한 확인
    for script in test_upload_download.sh test_directory_listing.sh test_redirects.sh; do
        if [ ! -x "$script" ]; then
            echo "🔧 $script 실행 권한 설정..."
            chmod +x "$script" 2>/dev/null || true
        fi
    done
    
    echo -e "${GREEN}🎯 테스트 대상 기능들:${NC}"
    echo "   1. 파일 업로드/다운로드"
    echo "   2. 디렉토리 리스팅"  
    echo "   3. HTTP 리다이렉트"
    echo ""
    
    # 1. 파일 업로드/다운로드 테스트
    if [ -f "upload_test.conf" ] && [ -f "test_upload_download.sh" ]; then
        run_test_suite "파일 업로드/다운로드" "upload_test.conf" "test_upload_download.sh" "8080"
    else
        echo -e "${YELLOW}⚠️  파일 업로드 테스트 건너뜀 (설정 파일 또는 스크립트 없음)${NC}"
    fi
    
    cleanup_servers
    
    # 2. 디렉토리 리스팅 테스트
    if [ -f "directory_test.conf" ] && [ -f "test_directory_listing.sh" ]; then
        run_test_suite "디렉토리 리스팅" "directory_test.conf" "test_directory_listing.sh" "8080"
    else
        echo -e "${YELLOW}⚠️  디렉토리 리스팅 테스트 건너뜀 (설정 파일 또는 스크립트 없음)${NC}"
    fi
    
    cleanup_servers
    
    # 3. 리다이렉트 테스트
    if [ -f "redirect_test.conf" ] && [ -f "test_redirects.sh" ]; then
        run_test_suite "HTTP 리다이렉트" "redirect_test.conf" "test_redirects.sh" "8080"
    else
        echo -e "${YELLOW}⚠️  리다이렉트 테스트 건너뜀 (설정 파일 또는 스크립트 없음)${NC}"
    fi
    
    # 최종 정리
    cleanup_servers
    
    print_test_header "테스트 완료"
    echo "종료 시간: $(date)"
    echo ""
    echo -e "${GREEN}🎉 모든 테스트가 완료되었습니다!${NC}"
    echo ""
    echo "📊 개별 테스트 재실행 방법:"
    echo "   ./webserv upload_test.conf & ./test_upload_download.sh"
    echo "   ./webserv directory_test.conf & ./test_directory_listing.sh"  
    echo "   ./webserv redirect_test.conf & ./test_redirects.sh"
    echo ""
    echo "🔧 수동 테스트 방법:"
    echo "   curl -X POST -d 'test data' http://127.0.0.1:8080/uploads/test.txt"
    echo "   curl http://127.0.0.1:8080/public/"
    echo "   curl -I http://127.0.0.1:8080/old-page"
}

# 인수에 따른 실행 모드
case "${1:-all}" in
    "upload")
        print_test_header "파일 업로드/다운로드 테스트만 실행"
        run_test_suite "파일 업로드/다운로드" "upload_test.conf" "test_upload_download.sh" "8080"
        cleanup_servers
        ;;
    "directory")
        print_test_header "디렉토리 리스팅 테스트만 실행"
        run_test_suite "디렉토리 리스팅" "directory_test.conf" "test_directory_listing.sh" "8080"
        cleanup_servers
        ;;
    "redirect")
        print_test_header "리다이렉트 테스트만 실행"
        run_test_suite "HTTP 리다이렉트" "redirect_test.conf" "test_redirects.sh" "8080"
        cleanup_servers
        ;;
    "all"|*)
        main
        ;;
esac