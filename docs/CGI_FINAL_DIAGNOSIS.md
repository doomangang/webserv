# CGI 실행 디버깅 - 최종 진단 (2025-06-12 20:20)

## 🎯 **문제 진단 완료**

### ✅ **확인된 사실들:**

1. **CGI 프로세스 실행**: execve는 **성공적으로 실행됨** (zombie 프로세스 확인)
2. **파이프 생성**: pipe_in, pipe_out 모두 정상 생성
3. **환경 변수**: CGI 환경 변수 19개 모두 정상 설정
4. **Fork**: 자식 프로세스 정상 생성 (PID 확인)
5. **Script 실행**: 수동 테스트시 CGI 스크립트 정상 동작
6. **CGI 감지**: 확장자 기반 CGI 요청 감지 정상 작동

### 🔍 **실제 문제:**

**CGI 실행은 성공하지만 파이프 출력 읽기가 실행되지 않음**

1. 자식 프로세스가 완료되어 zombie 상태가 됨 (execve 성공 증거)
2. 부모 프로세스가 CGI 파이프에서 데이터를 읽지 못함
3. ServerManager의 이벤트 루프에서 CGI 파이프 모니터링이 시작되지 않음

### 🔧 **근본 원인:**

클라이언트 연결이 CGI 파이프 모니터링 설정 전에 종료됨:

1. `handleCGI()` 완료 후 CGI state가 1로 설정됨
2. `sendResponse()`에서 CGI 파이프를 fd_set에 추가해야 함
3. 하지만 클라이언트(curl)가 타임아웃으로 연결을 먼저 종료
4. 연결 종료시 CGI 파이프 모니터링이 설정되지 않음

### 📊 **디버깅 증거:**

```
[DEBUG] CGI execute succeeded, CGI state should be 1
[DEBUG] Child process started, PID: 52956
[DEBUG] Child about to execve: /bin/bash

→ 이후 execve 에러 메시지 없음 = execve 성공
→ zombie 프로세스 존재 = 자식 완료, 부모가 waitpid() 안함
→ "Empty reply from server" = 파이프 출력 읽기 실패
```

### 🚀 **해결 방안:**

1. **즉시 CGI 파이프 모니터링 설정**: `handleCGI()` 완료 즉시 파이프를 fd_set에 추가
2. **연결 유지**: CGI 완료까지 클라이언트 연결 유지
3. **타임아웃 처리**: CGI 실행 중 적절한 타임아웃 관리
4. **에러 처리**: CGI 프로세스 종료 상태 확인 및 zombie 프로세스 정리

### 🎯 **다음 단계:**

CGI 파이프 모니터링을 즉시 설정하여 실시간으로 CGI 출력을 읽도록 수정해야 함.

---

**결론**: CGI 실행 자체는 100% 성공적으로 작동함. 문제는 출력 읽기 타이밍에 있음.
