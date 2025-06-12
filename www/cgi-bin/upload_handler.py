#!/usr/bin/env python3

import os
import sys
import cgi
import cgitb
from datetime import datetime

# CGI 에러 디버깅 활성화
cgitb.enable()

print("Content-Type: text/html")
print()

request_method = os.environ.get('REQUEST_METHOD', 'GET')

print("""<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>파일 업로드 처리 결과</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f8f9fa; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 40px; border-radius: 15px; box-shadow: 0 5px 15px rgba(0,0,0,0.1); }
        h1 { color: #2d3436; text-align: center; }
        .success { background: #d4edda; color: #155724; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .error { background: #f8d7da; color: #721c24; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .info { background: #e2e3e5; color: #383d41; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .file-info { background: #f8f9fa; padding: 15px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #6c5ce7; }
        pre { background: #f1f2f6; padding: 15px; border-radius: 5px; overflow-x: auto; }
        a { color: #007bff; text-decoration: none; }
        a:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <div class="container">
        <h1>📁 파일 업로드 처리 결과</h1>
""")

if request_method == 'POST':
    try:
        # 업로드된 파일 처리
        form = cgi.FieldStorage()
        
        if not form:
            print('<div class="error">업로드된 파일이 없습니다.</div>')
        else:
            uploaded_files = []
            file_count = 0
            
            # 폼 필드 처리
            for field_name in form.keys():
                field = form[field_name]
                
                if hasattr(field, 'filename') and field.filename:
                    file_count += 1
                    file_info = {
                        'field_name': field_name,
                        'filename': field.filename,
                        'content_type': field.type,
                        'size': len(field.value) if field.value else 0
                    }
                    uploaded_files.append(file_info)
                    
                    print(f'''
                    <div class="file-info">
                        <h3>파일 #{file_count}: {field.filename}</h3>
                        <p><strong>필드명:</strong> {field_name}</p>
                        <p><strong>파일명:</strong> {field.filename}</p>
                        <p><strong>Content-Type:</strong> {field.type or 'Unknown'}</p>
                        <p><strong>크기:</strong> {len(field.value) if field.value else 0} bytes</p>
                        <p><strong>업로드 시간:</strong> {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</p>
                    </div>
                    ''')
            
            if file_count > 0:
                print(f'<div class="success">✅ 총 {file_count}개의 파일이 성공적으로 업로드되었습니다!</div>')
                
                # 파일 저장 시뮬레이션 (실제로는 저장하지 않음)
                print('<div class="info">')
                print('<h3>💾 파일 저장 시뮬레이션</h3>')
                print('<p>실제 웹서버에서는 다음과 같은 위치에 파일이 저장됩니다:</p>')
                for i, file_info in enumerate(uploaded_files, 1):
                    safe_filename = file_info['filename'].replace(' ', '_').replace('..', '')
                    print(f'<p>파일 {i}: <code>/www/uploads/{safe_filename}</code></p>')
                print('</div>')
            else:
                print('<div class="error">❌ 업로드된 파일을 찾을 수 없습니다.</div>')
            
    except Exception as e:
        print(f'<div class="error">❌ 파일 업로드 처리 중 오류가 발생했습니다: {str(e)}</div>')

else:
    print('<div class="info">ℹ️ 이 페이지는 POST 메소드로만 접근할 수 있습니다.</div>')
    print('<p>파일 업로드 테스트는 <a href="/upload.html">업로드 페이지</a>에서 진행하세요.</p>')

# 환경 변수 정보 표시
print('<div class="info">')
print('<h3>🔧 요청 정보</h3>')
print(f'<p><strong>메소드:</strong> {request_method}</p>')
print(f'<p><strong>Content-Type:</strong> {os.environ.get("CONTENT_TYPE", "N/A")}</p>')
print(f'<p><strong>Content-Length:</strong> {os.environ.get("CONTENT_LENGTH", "N/A")}</p>')
print(f'<p><strong>서버:</strong> {os.environ.get("SERVER_NAME", "localhost")}:{os.environ.get("SERVER_PORT", "80")}</p>')
print(f'<p><strong>처리 시간:</strong> {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</p>')
print('</div>')

print('''
        <div style="text-align: center; margin-top: 40px;">
            <a href="/upload.html" style="background: #6c5ce7; color: white; padding: 12px 25px; border-radius: 20px; margin: 10px;">다시 업로드하기</a>
            <a href="/" style="background: #636e72; color: white; padding: 12px 25px; border-radius: 20px; margin: 10px;">메인 페이지로</a>
        </div>
    </div>
</body>
</html>''')