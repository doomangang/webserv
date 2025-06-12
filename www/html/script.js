// Webserv 테스트 페이지 JavaScript

document.addEventListener('DOMContentLoaded', function() {
    // 현재 시간 표시
    function updateCurrentTime() {
        const now = new Date();
        const timeString = now.toLocaleString('ko-KR', {
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });
        
        const timeElement = document.getElementById('current-time');
        if (timeElement) {
            timeElement.textContent = timeString;
        }
    }
    
    // User-Agent 표시
    function updateUserAgent() {
        const userAgentElement = document.getElementById('user-agent');
        if (userAgentElement) {
            userAgentElement.textContent = navigator.userAgent;
        }
    }
    
    // 초기 실행
    updateCurrentTime();
    updateUserAgent();
    
    // 매초마다 시간 업데이트
    setInterval(updateCurrentTime, 1000);
    
    // 페이지 로드 완료 메시지
    console.log('🚀 Webserv 테스트 페이지가 성공적으로 로드되었습니다!');
    console.log('JavaScript가 정상적으로 작동하고 있습니다.');
    
    // 링크 클릭 추적
    const links = document.querySelectorAll('a');
    links.forEach(link => {
        link.addEventListener('click', function(e) {
            console.log(`링크 클릭: ${this.href}`);
        });
    });
    
    // 간단한 상호작용 테스트
    let clickCount = 0;
    const title = document.querySelector('h1');
    if (title) {
        title.addEventListener('click', function() {
            clickCount++;
            this.style.transform = `rotate(${clickCount * 5}deg)`;
            console.log(`제목 클릭 횟수: ${clickCount}`);
            
            if (clickCount === 5) {
                alert('🎉 축하합니다! JavaScript가 정상적으로 작동합니다!');
            }
        });
        
        title.style.cursor = 'pointer';
        title.title = '클릭해보세요! (5번 클릭하면 특별한 일이...)';
    }
});

// 서버 연결 테스트 함수
async function testServerConnection() {
    try {
        const response = await fetch('/');
        if (response.ok) {
            console.log('✅ 서버 연결 정상');
            return true;
        } else {
            console.log('❌ 서버 응답 오류:', response.status);
            return false;
        }
    } catch (error) {
        console.log('❌ 서버 연결 실패:', error.message);
        return false;
    }
}

// 페이지 로드 후 서버 연결 테스트
window.addEventListener('load', function() {
    setTimeout(testServerConnection, 1000);
});