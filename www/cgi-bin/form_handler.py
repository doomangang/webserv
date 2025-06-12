#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs
import json

print("Content-Type: text/html")
print()

request_method = os.environ.get('REQUEST_METHOD', 'GET')
content_type = os.environ.get('CONTENT_TYPE', '')
content_length = int(os.environ.get('CONTENT_LENGTH', '0'))

print("""<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>CGI Form Handler</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .form-data { background: #e8f5e8; padding: 20px; margin: 20px 0; border-radius: 5px; }
        .method { background: #fff3cd; padding: 10px; margin: 10px 0; border-radius: 3px; }
        pre { background: #f8f9fa; padding: 15px; overflow-x: auto; }
    </style>
</head>
<body>
    <h1>📝 CGI Form Handler</h1>
    <div class="method">HTTP Method: <strong>{}</strong></div>
""".format(request_method))

data = {}

if request_method == 'POST':
    if content_length > 0:
        post_data = sys.stdin.buffer.read(content_length).decode('utf-8')
        if 'application/x-www-form-urlencoded' in content_type:
            data = parse_qs(post_data)
        else:
            data = {'raw_data': [post_data]}
        
        print('<div class="form-data">')
        print('<h2>POST 데이터:</h2>')
        print('<pre>')
        for key, values in data.items():
            for value in values:
                print(f"{key}: {value}")
        print('</pre>')
        print('</div>')
    else:
        print('<p>POST 데이터가 없습니다.</p>')

elif request_method == 'GET':
    query_string = os.environ.get('QUERY_STRING', '')
    if query_string:
        data = parse_qs(query_string)
        print('<div class="form-data">')
        print('<h2>GET 파라미터:</h2>')
        print('<pre>')
        for key, values in data.items():
            for value in values:
                print(f"{key}: {value}")
        print('</pre>')
        print('</div>')
    else:
        print('<p>GET 파라미터가 없습니다.</p>')

print("""
    <h2>🔧 테스트 양식</h2>
    <form method="POST" action="/cgi-bin/form_handler.py">
        <p>
            <label>이름: <input type="text" name="name" placeholder="홍길동"></label>
        </p>
        <p>
            <label>나이: <input type="number" name="age" placeholder="25"></label>
        </p>
        <p>
            <label>메시지: <textarea name="message" placeholder="안녕하세요!"></textarea></label>
        </p>
        <p>
            <button type="submit">POST 전송</button>
        </p>
    </form>
    
    <p><a href="/">← 메인 페이지로 돌아가기</a></p>
</body>
</html>""")