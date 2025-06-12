#!/bin/bash

echo "Content-Type: text/html"
echo ""

cat << 'EOF'
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>CGI Environment Variables</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        table { border-collapse: collapse; width: 100%; margin: 20px 0; }
        th, td { border: 1px solid #ddd; padding: 12px; text-align: left; }
        th { background-color: #f2f2f2; }
        tr:nth-child(even) { background-color: #f9f9f9; }
        .header { background: #007bff; color: white; padding: 20px; margin: -40px -40px 20px -40px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>🔧 CGI Environment Variables</h1>
        <p>Shell 스크립트로 생성된 환경변수 출력</p>
    </div>
    
    <table>
        <tr><th>변수명</th><th>값</th></tr>
EOF

env | sort | while IFS='=' read -r name value; do
    echo "        <tr><td><strong>$name</strong></td><td>$value</td></tr>"
done

cat << 'EOF'
    </table>
    
    <p><a href="/">← 메인 페이지로 돌아가기</a></p>
</body>
</html>
EOF