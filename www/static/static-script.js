// 정적 파일 서빙 테스트 JavaScript

document.addEventListener('DOMContentLoaded', function() {
    console.log('✅ 정적 JavaScript 파일이 성공적으로 로드되었습니다!');
    
    const testButton = document.getElementById('test-button');
    const jsResult = document.getElementById('js-result');
    
    if (testButton && jsResult) {
        testButton.addEventListener('click', function() {
            const now = new Date();
            const timestamp = now.toLocaleString('ko-KR');
            
            jsResult.innerHTML = `
                <strong>🎉 JavaScript 테스트 성공!</strong><br>
                클릭 시간: ${timestamp}<br>
                정적 JS 파일이 정상적으로 작동합니다.
            `;
            jsResult.style.display = 'block';
            
            // 버튼 텍스트 변경
            testButton.textContent = '✅ 테스트 완료!';
            testButton.style.background = '#27ae60';
            
            // 3초 후 원래대로 복원
            setTimeout(() => {
                testButton.textContent = '클릭하여 JavaScript 테스트';
                testButton.style.background = '#3498db';
            }, 3000);
        });
    }
    
    // 페이지 로드 시 CSS 로드 확인
    function checkCSSLoaded() {
        const testElement = document.createElement('div');
        testElement.style.display = 'none';
        testElement.className = 'styled-box';
        document.body.appendChild(testElement);
        
        const computedStyle = window.getComputedStyle(testElement);
        const hasGradient = computedStyle.background.includes('gradient') || 
                           computedStyle.backgroundImage.includes('gradient');
        
        document.body.removeChild(testElement);
        
        if (hasGradient) {
            console.log('✅ CSS 파일이 정상적으로 로드되었습니다!');
        } else {
            console.log('⚠️ CSS 파일 로드에 문제가 있을 수 있습니다.');
        }
    }
    
    // 페이지 요소들에 애니메이션 효과
    function addAnimations() {
        const sections = document.querySelectorAll('.test-section');
        sections.forEach((section, index) => {
            section.style.opacity = '0';
            section.style.transform = 'translateY(30px)';
            
            setTimeout(() => {
                section.style.transition = 'all 0.6s ease';
                section.style.opacity = '1';
                section.style.transform = 'translateY(0)';
            }, index * 200);
        });
    }
    
    // 링크 클릭 모니터링
    function monitorLinks() {
        const links = document.querySelectorAll('a[href$=".css"], a[href$=".js"], a[href$=".html"], a[href$=".txt"], a[href$=".json"]');
        links.forEach(link => {
            link.addEventListener('click', function(e) {
                console.log(`파일 요청: ${this.href}`);
                
                // 파일 타입별 아이콘 추가
                const href = this.href;
                if (href.endsWith('.css')) this.innerHTML = '🎨 ' + this.innerHTML;
                else if (href.endsWith('.js')) this.innerHTML = '⚡ ' + this.innerHTML;
                else if (href.endsWith('.html')) this.innerHTML = '📄 ' + this.innerHTML;
                else if (href.endsWith('.txt')) this.innerHTML = '📝 ' + this.innerHTML;
                else if (href.endsWith('.json')) this.innerHTML = '📊 ' + this.innerHTML;
            });
        });
    }
    
    // 초기화 함수들 실행
    setTimeout(checkCSSLoaded, 100);
    setTimeout(addAnimations, 200);
    monitorLinks();
    
    // 파일 존재 여부 확인 (비동기)
    async function checkFileExists(url) {
        try {
            const response = await fetch(url, { method: 'HEAD' });
            return response.ok;
        } catch (error) {
            return false;
        }
    }
    
    // 테스트 파일들 확인
    const testFiles = ['test.html', 'test.txt', 'test.json'];
    testFiles.forEach(async (file) => {
        const exists = await checkFileExists(file);
        const link = document.querySelector(`a[href="${file}"]`);
        if (link) {
            if (exists) {
                link.style.color = '#27ae60';
                link.title = '파일이 존재합니다';
            } else {
                link.style.color = '#e74c3c';
                link.title = '파일이 존재하지 않습니다';
            }
        }
    });
});