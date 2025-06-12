#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs
from datetime import datetime

print("Content-Type: text/html")
print()

query_string = os.environ.get('QUERY_STRING', '')
request_method = os.environ.get('REQUEST_METHOD', 'GET')

print("""<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>CGI Hello World</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .info { background: #f0f0f0; padding: 20px; margin: 20px 0; }
        .highlight { color: #007bff; font-weight: bold; }
    </style>
</head>
<body>
    <h1>🐍 Python CGI Hello World!</h1>
    <p>현재 시간: <span class="highlight">{}</span></p>
    
    <div class="info">
        <h2>CGI 환경 변수</h2>
        <p><strong>REQUEST_METHOD:</strong> {}</p>
        <p><strong>QUERY_STRING:</strong> {}</p>
        <p><strong>SERVER_NAME:</strong> {}</p>
        <p><strong>SERVER_PORT:</strong> {}</p>
        <p><strong>HTTP_USER_AGENT:</strong> {}</p>
    </div>
    
    <div class="info">
        <h2>쿼리 파라미터 테스트</h2>
        <p>URL에 ?name=홍길동&age=25 같은 파라미터를 추가해보세요!</p>
""".format(
    datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
    request_method,
    query_string,
    os.environ.get('SERVER_NAME', 'localhost'),
    os.environ.get('SERVER_PORT', '80'),
    os.environ.get('HTTP_USER_AGENT', 'Unknown')
))

if query_string:
    params = parse_qs(query_string)
    print("        <ul>")
    for key, values in params.items():
        for value in values:
            print(f"            <li><strong>{key}:</strong> {value}</li>")
    print("        </ul>")
else:
    print("        <p>쿼리 파라미터가 없습니다.</p>")

print("""    </div>
    
    <p><a href="/">← 메인 페이지로 돌아가기</a></p>
</body>
</html>""")