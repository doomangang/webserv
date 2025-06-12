# HTTP Method Support Refactoring

## 🎯 Overview

웹서버의 HTTP 메서드 지원을 개선하고 GET 요청이 405 Method Not Allowed 에러를 반환하던 문제를 해결했습니다. 또한 POST와 DELETE 메서드가 실제 의미 있는 동작을 수행하도록 구현했습니다.

## 🐛 Issues Fixed

### 1. GET 요청 405 에러 문제
- **문제**: GET 요청에 대해서도 405 Method Not Allowed 에러가 발생
- **원인**: 제거된 HTTP 메서드 enum 값들(PUT, HEAD, EMPTY)이 여전히 코드에서 참조됨
- **해결**: 관련 코드를 모두 정리하고 지원하는 메서드만 처리하도록 수정

### 2. 단순한 JSON 응답 문제
- **문제**: POST와 DELETE가 실제 동작 없이 단순한 JSON 응답만 반환
- **해결**: 각 메서드가 실제 파일 시스템 작업을 수행하도록 구현

## 🔧 Changes Made

### 📁 Modified Files

#### `src/HttpUtils.cpp`
- `stringToMethod()`: PUT, HEAD, EMPTY 메서드 처리 제거
- `methodToString()`: 지원하는 메서드만 처리하도록 수정

#### `src/ConfigParser.cpp`
- `parseMethodsDirective()`: 지원되지 않는 메서드는 무시하도록 수정
- PUT, HEAD, EMPTY 메서드 파싱 로직 제거

#### `src/RequestParser.cpp`
- 지원되지 않는 메서드에 대해 400 → 405 에러 코드로 변경

#### `src/Request.cpp`
- `getMethodStr()`: 제거된 enum 값들 처리 제거

#### `src/Client.cpp`
- 메서드별 분기 처리 로직 추가
- `handleGetRequest()`: 정적 파일 서빙 (기존 로직)
- `handlePostRequest()`: 실제 파일 생성/업로드 기능
- `handleDeleteRequest()`: 실제 파일 삭제 기능 (보안 제약 포함)
- `handleFileUpload()`: 파일 업로드 처리

#### `inc/Client.hpp`
- 새로운 메서드별 처리 함수 선언 추가

#### `configs/default.conf`
- 현재 www 구조에 맞게 경로 수정 (`/tours` → `/static`)
- 웹서버 관례에 따른 메서드 제한 적용
- 업로드 경로 추가

### 🏗️ Infrastructure Changes

#### Directory Structure
```bash
www/
├── uploads/          # 새로 생성된 업로드 디렉토리
├── html/            # 기존 메인 콘텐츠
├── static/          # 정적 파일 (기존)
└── cgi-bin/         # CGI 스크립트 (기존)
```

## ✨ New Features

### 1. **실제 파일 작업 지원**

#### POST 요청
- **일반 경로**: 요청 데이터를 파일로 저장
  - 메타데이터 포함 (타임스탬프, 경로, 콘텐츠 길이)
  - 파일명: `post_data_YYYY-MM-DD HH:MM:SS.txt`
- **업로드 경로**: 파일 업로드 기능
  - 업로드된 콘텐츠를 바이너리로 저장
  - 파일명: `upload_YYYY-MM-DD HH:MM:SS.txt`

#### DELETE 요청
- **보안 제약**: `/static/`과 `/upload/` 경로에서만 허용
- **실제 삭제**: 파일 시스템에서 파일 제거
- **권한 검사**: 디렉토리 삭제 방지, 권한 확인

### 2. **향상된 에러 처리**
- **405 Method Not Allowed**: 지원되지 않는 메서드 (PUT, HEAD 등)
- **403 Forbidden**: 안전하지 않은 경로에서의 삭제 시도
- **JSON 형태 에러 응답**: 구조화된 에러 메시지

### 3. **경로별 메서드 제한 (웹서버 관례 준수)**

| Location | Allowed Methods | Purpose |
|----------|----------------|---------|
| `/` | GET, POST | 메인 페이지, 폼 처리 |
| `/static/` | GET, DELETE | 정적 파일 서빙, 관리 |
| `/upload/` | GET, POST, DELETE | 파일 업로드, 관리 |
| `/cgi-bin/` | GET, POST | 동적 스크립트 실행 |

## 🧪 Testing

### Test Cases

#### ✅ GET 요청
```bash
curl -v http://127.0.0.1:8002/
# → 200 OK, HTML 콘텐츠 반환

curl -v http://127.0.0.1:8002/static/
# → 200 OK, 디렉토리 리스팅 반환
```

#### ✅ POST 요청
```bash
curl -v -X POST -d 'test data' http://127.0.0.1:8002/
# → 201 Created, 파일 생성 및 JSON 응답

curl -v -X POST -d 'upload content' http://127.0.0.1:8002/upload/
# → 201 Created, 파일 업로드 및 JSON 응답
```

#### ✅ DELETE 요청
```bash
curl -v -X DELETE http://127.0.0.1:8002/static/test_file.txt
# → 200 OK, 파일 삭제 및 JSON 응답

curl -v -X DELETE http://127.0.0.1:8002/index.html
# → 403 Forbidden, 안전하지 않은 경로
```

#### ✅ 지원되지 않는 메서드
```bash
curl -v -X PUT http://127.0.0.1:8002/
# → 405 Method Not Allowed

curl -v -X HEAD http://127.0.0.1:8002/
# → 405 Method Not Allowed
```

#### ✅ 경로별 메서드 제한
```bash
curl -v -X DELETE http://127.0.0.1:8002/
# → 405 Method Not Allowed (루트 경로에서 DELETE 금지)

curl -v -X POST http://127.0.0.1:8002/static/
# → 405 Method Not Allowed (정적 경로에서 POST 금지)
```

## 🛡️ Security Improvements

1. **경로 기반 권한 제어**: 중요 파일 보호
2. **메서드 제한**: 각 경로의 목적에 맞는 메서드만 허용
3. **삭제 권한 제한**: 특정 디렉토리에서만 삭제 허용
4. **파일 타입 검증**: 디렉토리 삭제 방지

## 📈 Performance & Reliability

- **에러 처리 개선**: 명확한 HTTP 상태 코드 사용
- **로깅 강화**: 요청 처리 과정 상세 로깅
- **메모리 안전성**: 적절한 파일 핸들링 및 리소스 정리

## 🔄 Backward Compatibility

- 기존 GET 요청 동작 유지
- 기존 설정 파일 구조 호환성 유지
- 기존 정적 파일 서빙 기능 보존

## 📝 Configuration Changes

### Before
```conf
location /tours {
    autoindex on;
    index tours1.html;
    allow_methods GET POST PUT HEAD;
}
```

### After
```conf
location /static {
    autoindex on;
    allow_methods GET DELETE;
    root ./www/static;
}

location /upload {
    allow_methods GET POST DELETE;
    upload_store ./www/uploads;
}
```

## 🎉 Results

- ✅ GET 요청 405 에러 완전 해결
- ✅ POST/DELETE 메서드 실제 기능 구현
- ✅ 웹서버 보안 및 관례 준수
- ✅ 구조화된 에러 응답 제공
- ✅ 파일 시스템 작업 지원

## 📚 Code Quality

- **함수 분리**: 메서드별 처리 로직 분리
- **에러 처리**: 체계적인 에러 처리 및 응답
- **설정 검증**: 안전한 설정 파일 파싱
- **코드 정리**: 사용하지 않는 enum 값 제거

---

**작업 기간**: 2025년 6월 12일  
**영향 범위**: HTTP 메서드 처리, 파일 시스템 작업, 보안 정책  
**테스트 상태**: 모든 테스트 케이스 통과 ✅
