// Parallax Documentation Interactive Simulator & Code Copy
document.addEventListener('DOMContentLoaded', () => {
    // Simulator State
    const mockWindow = document.getElementById('mockWindow');
    const mockNav = document.getElementById('mockNav');
    const sideButtons = document.querySelectorAll('[data-side]');
    const tierButtons = document.querySelectorAll('[data-tier]');
    const toggleBtn = document.getElementById('simToggleBtn');

    let currentSide = 'left';
    let currentTier = 'standard';
    let isOpen = true;

    const tierDimensions = {
        compact: { w: '440px', h: '290px' },
        standard: { w: '520px', h: '340px' },
        expanded: { w: '600px', h: '370px' }
    };

    function updateSimulator() {
        if (!mockWindow) return;

        if (!isOpen) {
            mockWindow.style.opacity = '0';
            mockWindow.style.transform = 'scale(0.92)';
            mockWindow.style.pointerEvents = 'none';
            return;
        }

        mockWindow.style.opacity = '1';
        mockWindow.style.transform = 'scale(1)';
        mockWindow.style.pointerEvents = 'auto';

        const dim = tierDimensions[currentTier];
        mockWindow.style.width = dim.w;
        mockWindow.style.height = dim.h;

        if (currentSide === 'left') {
            mockWindow.style.flexDirection = 'row';
            mockNav.style.flexDirection = 'column';
            mockNav.style.width = '130px';
            mockNav.style.height = '100%';
            mockNav.style.borderRight = '1px solid var(--border)';
            mockNav.style.borderBottom = 'none';
            mockWindow.insertBefore(mockNav, mockWindow.firstChild);
        } else if (currentSide === 'right') {
            mockWindow.style.flexDirection = 'row';
            mockNav.style.flexDirection = 'column';
            mockNav.style.width = '130px';
            mockNav.style.height = '100%';
            mockNav.style.borderLeft = '1px solid var(--border)';
            mockNav.style.borderRight = 'none';
            mockNav.style.borderBottom = 'none';
            mockWindow.appendChild(mockNav);
        } else if (currentSide === 'top') {
            mockWindow.style.flexDirection = 'column';
            mockNav.style.flexDirection = 'row';
            mockNav.style.width = '100%';
            mockNav.style.height = '42px';
            mockNav.style.borderBottom = '1px solid var(--border)';
            mockNav.style.borderRight = 'none';
            mockWindow.insertBefore(mockNav, mockWindow.firstChild);
        } else if (currentSide === 'bottom') {
            mockWindow.style.flexDirection = 'column';
            mockNav.style.flexDirection = 'row';
            mockNav.style.width = '100%';
            mockNav.style.height = '42px';
            mockNav.style.borderTop = '1px solid var(--border)';
            mockNav.style.borderBottom = 'none';
            mockWindow.appendChild(mockNav);
        }
    }

    sideButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            sideButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            currentSide = btn.getAttribute('data-side');
            updateSimulator();
        });
    });

    tierButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            tierButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            currentTier = btn.getAttribute('data-tier');
            updateSimulator();
        });
    });

    if (toggleBtn) {
        toggleBtn.addEventListener('click', () => {
            isOpen = !isOpen;
            toggleBtn.textContent = isOpen ? 'Simulate Close (Insert / RShift)' : 'Simulate Open (Insert / RShift)';
            updateSimulator();
        });
    }

    updateSimulator();

    // Smooth navigation anchor scrolling
    document.querySelectorAll('.nav-link').forEach(link => {
        link.addEventListener('click', (e) => {
            const href = link.getAttribute('href');
            if (href && href.startsWith('#')) {
                e.preventDefault();
                document.querySelectorAll('.nav-link').forEach(l => l.classList.remove('active'));
                link.classList.add('active');
                const target = document.querySelector(href);
                if (target) {
                    target.scrollIntoView({ behavior: 'smooth' });
                }
            }
        });
    });
});
