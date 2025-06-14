#!/bin/bash

echo "=== 리다이렉트 테스트 ==="

# 리다이렉트 테스트용 설정
cat > redirect_test.conf << 'EOF'
server {
    listen 127.0.0.1:8080;
    server_name redirect.local;
    root ./www/redirect_test;
    index index.html;
    
    location / {
        allow_methods GET POST;
        autoindex on;
    }
    
    # 301 영구 리다이렉트
    location /old-page {
        return 301 http://127.0.0.1:8080/new-page;
    }
    
    # 302 임시 리다이렉트
    location /temp {
        return 302 http://127.0.0.1:8080/temporary-moved;
    }
    
    # 외부 사이트로 리다이렉트
    location /external {
        return 301 https://www.example.com/;
    }
    
    # 상대 경로 리다이렉트
    location /redirect-relative {
        return 302 /target-page;
    }
    
    # 쿼리 파라미터 포함 리다이렉트
    location /search {
        return 301 http://127.0.0.1:8080/results?source=redirect;
    }
    
    # 리다이렉트 대상 페이지들
    location /new-page {
        allow_methods GET;
        autoindex off;
    }
    
    location /temporary-moved {
        allow_methods GET;
        autoindex off;
    }
    
    location /target-page {
        allow_methods GET;
        autoindex off;
    }
    
    location /results {
        allow_methods GET;
        autoindex off;
    }
}
EOF

echo "Created redirect_test.conf"

# 테스트용 페이지 생성
echo "Creating test pages..."

mkdir -p ./www/redirect_test

cat > ./www/redirect_test/index.html << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>Redirect Test Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .section { margin: 20px 0; padding: 20px; border: 1px solid #ddd; }
        .redirect-link { display: block; margin: 10px 0; padding: 10px; background: #f0f0f0; text-decoration: none; }
        .redirect-link:hover { background: #e0e0e0; }
        .code { background: #f5f5f5; padding: 2px 4px; font-family: monospace; }
    </style>
</head>
<body>
    <h1>🔄 Redirect Test Server</h1>
    
    <div class="section">
        <h2>Test Redirects (Click to Test)</h2>
        <a href="/old-page" class="redirect-link">
            🔗 /old-page → /new-page (301 Permanent)
        </a>
        <a href="/temp" class="redirect-link">
            🔗 /temp → /temporary-moved (302 Temporary)
        </a>
        <a href="/external" class="redirect-link">
            🔗 /external → https://www.example.com/ (301 External)
        </a>
        <a href="/redirect-relative" class="redirect-link">
            🔗 /redirect-relative → /target-page (302 Relative)
        </a>
        <a href="/search" class="redirect-link">
            🔗 /search → /results?source=redirect (301 with Query)
        </a>
    </div>
    
    <div class="section">
        <h2>Target Pages (Direct Access)</h2>
        <a href="/new-page" class="redirect-link">📄 /new-page</a>
        <a href="/temporary-moved" class="redirect-link">📄 /temporary-moved</a>
        <a href="/target-page" class="redirect-link">📄 /target-page</a>
        <a href="/results" class="redirect-link">📄 /results</a>
    </div>
    
    <div class="section">
        <h2>Test with curl</h2>
        <pre>
# Test redirects with curl
curl -I http://127.0.0.1:8080/old-page           # Should return 301
curl -I http://127.0.0.1:8080/temp               # Should return 302
curl -L http://127.0.0.1:8080/old-page           # Follow redirect (-L flag)
curl -w "%{redirect_url}\n" http://127.0.0.1:8080/old-page
        </pre>
    </div>
</body>
</html>
EOF

# 리다이렉트 대상 페이지들
cat > ./www/redirect_test/new-page.html << 'EOF'
<!DOCTYPE html>
<html>
<head><title>New Page</title></head>
<body>
    <h1>✅ New Page</h1>
    <p>You successfully reached the new page via 301 redirect!</p>
    <p>This page replaced the old page permanently.</p>
    <a href="/">← Back to home</a>
</body>
</html>
EOF

cat > ./www/redirect_test/temporary-moved.html << 'EOF'
<!DOCTYPE html>
<html>
<head><title>Temporary Page</title></head>
<body>
    <h1>🔄 Temporary Page</h1>
    <p>You reached this page via 302 redirect!</p>
    <p>This is a temporary redirect - the original page may come back.</p>
    <a href="/">← Back to home</a>
</body>
</html>
EOF

cat > ./www/redirect_test/target-page.html << 'EOF'
<!DOCTYPE html>
<html>
<head><title>Target Page</title></head>
<body>
    <h1>🎯 Target Page</h1>
    <p>You reached this page via relative redirect!</p>
    <a href="/">← Back to home</a>
</body>
</html>
EOF

cat > ./www/redirect_test/results.html << 'EOF'
<!DOCTYPE html>
<html>
<head><title>Results Page</title></head>
<body>
    <h1>📊 Results Page</h1>
    <p>You reached this page via redirect with query parameters!</p>
    <p>Check the URL for: ?source=redirect</p>
    <a href="/">← Back to home</a>
</body>
</html>
EOF

echo "Test pages created."

# 자동 테스트 스크립트
cat > test_redirects.sh << 'EOF'
#!/bin/bash

echo "=== 리다이렉트 자동 테스트 ==="

# 서버 실행 확인
if ! nc -z 127.0.0.1 8080 2>/dev/null; then
    echo "❌ 서버가 실행되지 않았습니다."
    echo "서버를 시작하세요: ./webserv redirect_test.conf"
    exit 1
fi

echo "✅ 서버가 실행 중입니다."
echo ""

# Test 1: 301 영구 리다이렉트
echo "🧪 Test 1: 301 영구 리다이렉트 (/old-page)"
response_301=$(curl -s -I "http://127.0.0.1:8080/old-page")
status_301=$(echo "$response_301" | head -n1 | grep -o "301")
location_301=$(echo "$response_301" | grep -i "Location:" | awk '{print $2}' | tr -d '\r\n')

echo "Status: $status_301, Location: $location_301"

if [ "$status_301" = "301" ] && echo "$location_301" | grep -q "new-page"; then
    echo "✅ 301 리다이렉트 성공"
else
    echo "❌ 301 리다이렉트 실패"
    echo "Full response:"
    echo "$response_301"
fi

# Test 2: 302 임시 리다이렉트
echo -e "\n🧪 Test 2: 302 임시 리다이렉트 (/temp)"
response_302=$(curl -s -I "http://127.0.0.1:8080/temp")
status_302=$(echo "$response_302" | head -n1 | grep -o "302")
location_302=$(echo "$response_302" | grep -i "Location:" | awk '{print $2}' | tr -d '\r\n')

echo "Status: $status_302, Location: $location_302"

if [ "$status_302" = "302" ] && echo "$location_302" | grep -q "temporary-moved"; then
    echo "✅ 302 리다이렉트 성공"
else
    echo "❌ 302 리다이렉트 실패"
    echo "Full response:"
    echo "$response_302"
fi

# Test 3: 외부 사이트 리다이렉트
echo -e "\n🧪 Test 3: 외부 사이트 리다이렉트 (/external)"
response_external=$(curl -s -I "http://127.0.0.1:8080/external")
status_external=$(echo "$response_external" | head -n1 | grep -o "301")
location_external=$(echo "$response_external" | grep -i "Location:" | awk '{print $2}' | tr -d '\r\n')

echo "Status: $status_external, Location: $location_external"

if [ "$status_external" = "301" ] && echo "$location_external" | grep -q "example.com"; then
    echo "✅ 외부 리다이렉트 성공"
else
    echo "❌ 외부 리다이렉트 실패"
fi

# Test 4: 상대 경로 리다이렉트
echo -e "\n🧪 Test 4: 상대 경로 리다이렉트 (/redirect-relative)"
response_relative=$(curl -s -I "http://127.0.0.1:8080/redirect-relative")
status_relative=$(echo "$response_relative" | head -n1 | grep -o "302")
location_relative=$(echo "$response_relative" | grep -i "Location:" | awk '{print $2}' | tr -d '\r\n')

echo "Status: $status_relative, Location: $location_relative"

if [ "$status_relative" = "302" ] && echo "$location_relative" | grep -q "target-page"; then
    echo "✅ 상대 경로 리다이렉트 성공"
else
    echo "❌ 상대 경로 리다이렉트 실패"
fi

# Test 5: 쿼리 파라미터 포함 리다이렉트
echo -e "\n🧪 Test 5: 쿼리 파라미터 리다이렉트 (/search)"
response_query=$(curl -s -I "http://127.0.0.1:8080/search")
status_query=$(echo "$response_query" | head -n1 | grep -o "301")
location_query=$(echo "$response_query" | grep -i "Location:" | awk '{print $2}' | tr -d '\r\n')

echo "Status: $status_query, Location: $location_query"

if [ "$status_query" = "301" ] && echo "$location_query" | grep -q "results.*source=redirect"; then
    echo "✅ 쿼리 파라미터 리다이렉트 성공"
else
    echo "❌ 쿼리 파라미터 리다이렉트 실패"
fi

# Test 6: 리다이렉트 따라가기 (-L 옵션)
echo -e "\n🧪 Test 6: 리다이렉트 자동 따라가기"
final_content=$(curl -s -L "http://127.0.0.1:8080/old-page")

if echo "$final_content" | grep -q "New Page.*301 redirect"; then
    echo "✅ 리다이렉트 자동 따라가기 성공"
else
    echo "❌ 리다이렉트 자동 따라가기 실패"
fi

# Test 7: 리다이렉트되지 않는 정상 페이지
echo -e "\n🧪 Test 7: 정상 페이지 (리다이렉트 없음)"
response_normal=$(curl -s -I "http://127.0.0.1:8080/")
status_normal=$(echo "$response_normal" | head -n1 | grep -o "200")

if [ "$status_normal" = "200" ]; then
    echo "✅ 정상 페이지 200 응답"
else
    echo "❌ 정상 페이지 응답 문제"
fi

# Test 8: 존재하지 않는 리다이렉트 경로
echo -e "\n🧪 Test 8: 존재하지 않는 경로"
response_404=$(curl -s -I "http://127.0.0.1:8080/nonexistent")
status_404=$(echo "$response_404" | head -n1 | grep -o "404")

if [ "$status_404" = "404" ]; then
    echo "✅ 존재하지 않는 경로 404 응답"
else
    echo "❌ 존재하지 않는 경로 응답 문제"
fi

echo -e "\n=== 리다이렉트 테스트 완료 ==="
echo ""
echo "📋 수동 테스트 명령어:"
echo "  # 헤더만 확인 (리다이렉트 응답)"
echo "  curl -I http://127.0.0.1:8080/old-page"
echo "  curl -I http://127.0.0.1:8080/temp"
echo "  "
echo "  # 리다이렉트 따라가기"  
echo "  curl -L http://127.0.0.1:8080/old-page"
echo "  "
echo "  # 리다이렉트 URL만 출력"
echo "  curl -w '%{redirect_url}\n' -o /dev/null -s http://127.0.0.1:8080/old-page"
echo ""
echo "🌐 브라우저에서 확인:"
echo "  http://127.0.0.1:8080/old-page"
echo "  http://127.0.0.1:8080/temp"

if [ -z "$status_301" ] || [ -z "$status_302" ]; then
    echo ""
    echo "⚠️  리다이렉트가 작동하지 않는 경우 확인사항:"
    echo "1. Location 클래스에 return 지시어 처리가 구현되어 있는가?"
    echo "2. Response::setStatusCode()와 setHeader()가 올바르게 작동하는가?"
    echo "3. Client::handleRedirect()가 호출되는가?"
    echo "4. 설정 파일의 return 지시어가 올바르게 파싱되는가?"
fi

EOF

chmod +x test_redirects.sh

echo ""
echo "🚀 리다이렉트 테스트 실행:"
echo "1. ./webserv redirect_test.conf"
echo "2. ./test_redirects.sh"
echo ""
echo "🔍 리다이렉트가 안 되는 경우 체크포인트:"
echo "   - Location::hasRedirect() 구현 확인"
echo "   - Client::handleRedirect() 호출 확인"  
echo "   - Response HTTP 헤더 설정 확인"
echo "   - ConfigParser의 return 지시어 파싱 확인"