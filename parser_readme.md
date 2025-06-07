# config parser
- 웹서버 실행 시 동작 정의해주는 설정 파일
> config 예시
```
server {
    # 서버가 바인딩할 호스트와 포트
    listen 127.0.0.1:8080;

    # 이 서버를 처리할 도메인 이름(들)
    server_name example.com www.example.com;

    # 요청 URI 최대 길이 (바이트 단위)
    request_uri_limit 2048;

    # 요청 헤더 최대 크기 (바이트 단위)
    request_header_limit 8192;

    # 클라이언트 본문(파일 업로드 등) 최대 크기 (바이트 단위)
    client_body_limit 1048576;

    # 문서 루트(서버 레벨)
    root /var/www/html;

    # 디렉터리 요청 시 기본으로 찾을 index 파일들
    index index.html index.htm;

    # 디렉터리 인덱스 기능 켜기(on) 또는 끄기(off)
    autoindex on;

    # 파일 업로드 시 저장할 디렉터리
    upload_store /tmp/uploads;

    # 기본 에러 페이지 (상태 코드 → 반환할 경로)
    error_page 404 /404.html;
    error_page 500 /50x.html;

    # -------------------------------------------------------
    # Location 블록 #1: /images 요청 처리
    # -------------------------------------------------------
    location /images {
        # 이 Location에 대한 별도 문서 루트
        root /var/www/images;

        # 디렉터리 요청 시 기본으로 찾을 index 파일
        index img.html;

        # 허용할 HTTP 메소드 목록
        allow_methods GET POST;

        # CGI 확장자 설정 (테스트 예시로 Set만 사용하지만, 실제로는 맵으로 저장)
        cgi_extension .php /usr/bin/php-cgi;

        # 디렉터리 인덱스 기능 끄기
        autoindex off;

        # 리다이렉트 설정: 302 코드로 지정된 URL을 반환
        return 302 http://redirect.local;

        # 이 Location에서 업로드된 파일 저장 경로
        upload_store /var/www/images/uploads;
    }

    # -------------------------------------------------------
    # Location 블록 #2: /upload 요청 처리
    # -------------------------------------------------------
    location /upload {
        # 업로드 전용 디렉터리
        root /var/www/upload;

        # 업로드 API 엔드포인트이므로 인덱스 파일 없이 POST만 허용
        allow_methods POST;

        # 업로드된 파일을 저장할 별도 경로
        upload_store /var/www/upload/files;

        # autoindex는 의미가 없으므로 기본값(false)로 두거나 생략
        autoindex off;
    }
}
```

## conf 파싱 후 최종적으로 생기는 변수들에 대한 설명
config 클래스 = .conf 파일 단위

### config mem attribute
```
std::vector<Server> _servers;
std::string _software_name;
std::string _software_version;
std::string _http_version;
std::string _cgi_version;
char**      _base_env;
```
- std::vector<Server> _servers;
    -> config 파일 내부는 1개 이상의 서버 블록이 존재함. 서버 클래스를 벡터화하여 가지고 있다.

- header(_software_name, _software_version, _http_version, _cgi_version)
    -> 서버 블록 상단의 헤더 내용인데, 서브젝트에서 필수로 언급되지 않아 현재는 무시하고 변수 사용하지 않음

- _base_env
    -> main 에서 전달받은 env

### server mem attribute
```
ServerManager*              _manager;
std::vector<std::string>    _server_names;
std::string                 _host;
int                         _port;
int                         _fd;
int                         _request_uri_limit_size;
int                         _request_header_limit_size;
int                         _limit_client_body_size;
std::string                 _root_path;             // 새로 추가
std::vector<std::string>    _index_files;           // 새로 추가
bool                        _autoindex;             // 새로 추가
std::string                 _upload_store;          // 새로 추가
bool                        _has_upload_store;      // 새로 추가
std::string                 _default_error_page;
std::map<int, std::string>  _error_pages;
Config*                     _config;
std::vector<Location>       _locations;
std::map<int, Connection>   _connections;
std::queue<Response>        _responses;
```

- 서버 레벨 설정
    listen <host>:<port>; → _host, _port
    server_name <이름1> <이름2> ...; → _server_names
    request_uri_limit <바이트>; → _request_uri_limit_size
    request_header_limit <바이트>; → _request_header_limit_size
    client_body_limit <바이트>; → _limit_client_body_size
    root <경로>; → _root_path
    index <파일1> <파일2> ...; → _index_files
    autoindex <on|off>; → _autoindex
    upload_store <경로>; → _upload_store, _has_upload_store = true
    error_page <코드> <경로>; → _error_pages[코드] = 경로

    서버 레벨에서 설정된 _root_path, _index_files, _autoindex 같은 값들은 디폴트, 각 location 블록을 파싱할 때 그 안에서 같은 이름의 디렉티브가 나오면 해당 Location 객체의 멤버를 덮어씀.

### Locations
```
std::string              _uri;
std::string              _root_path;
std::set<std::string>    _allow_methods;
std::set<std::string>    _index_files;
std::set<std::string>    _cgi_extensions;
bool                     _autoindex;
bool                     _has_redirect;
int                      _redirect_code;
std::string              _redirect_url;
bool                     _has_upload_store;
std::string              _upload_store;
```

- Location 블록 내부 설정
    location <URI> { ... } → 하나의 Location 객체 생성, _uri = <URI>
    root <경로>; → _root_path
    index <파일1> <파일2> ...; → _index_files
    allow_methods <메소드1> <메소드2> ...; → _allow_methods
    cgi_extension <확장자> <실행파일경로>; → _cgi_extensions (테스트 코드에서는 Set만 사용했지만, 실제 구현 시 맵 형태로 저장)
    autoindex <on|off>; → _autoindex
    return <코드> <URL>; → _has_redirect = true, _redirect_code, _redirect_url
    upload_store <경로>; → _upload_store, _has_upload_store = true

## Request parser
server run 시에 connection 의 mainprocess 내부에서 ReadClient 를 호출. 소켓에 바이트 스트림으로 쌓인 http 요청을 읽어오는 것이 역할



