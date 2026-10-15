// Parallax Documentation Interactive Simulator & Phase 2 Widget Playground
document.addEventListener('DOMContentLoaded', () => {
    // -------------------------------------------------------------------------
    // 1. Layout & Motion Simulator
    // -------------------------------------------------------------------------
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

    // -------------------------------------------------------------------------
    // 2. Phase 2 Interactive Widget Playground Handlers
    // -------------------------------------------------------------------------

    // Toggles (Pill switches)
    const toggleBbox = document.getElementById('toggle-bbox');
    if (toggleBbox) {
        toggleBbox.addEventListener('click', () => {
            toggleBbox.classList.toggle('active');
        });
    }

    const toggleSound = document.getElementById('toggle-sound');
    if (toggleSound) {
        toggleSound.addEventListener('click', () => {
            toggleSound.classList.toggle('active');
        });
    }

    // Checkbox
    const checkSnap = document.getElementById('check-snap');
    if (checkSnap) {
        checkSnap.addEventListener('click', () => {
            checkSnap.classList.toggle('active');
            checkSnap.innerHTML = checkSnap.classList.contains('active')
                ? `<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3"><polyline points="20 6 9 17 4 12"/></svg>`
                : '';
        });
    }

    // Keybind Button
    const keyBbox = document.getElementById('key-bbox');
    let isBinding = false;
    if (keyBbox) {
        keyBbox.addEventListener('click', (e) => {
            e.stopPropagation();
            isBinding = true;
            keyBbox.classList.add('listening');
            keyBbox.textContent = '[ ... ]';
        });

        window.addEventListener('keydown', (e) => {
            if (isBinding) {
                e.preventDefault();
                let keyStr = e.key.toUpperCase();
                if (keyStr === ' ') keyStr = 'SPACE';
                if (keyStr === 'ESCAPE') keyStr = 'NONE';
                keyBbox.textContent = `[ ${keyStr} ]`;
                keyBbox.classList.remove('listening');
                isBinding = false;
            }
        });
    }

    // Color Swatches (Cycle theme colors on click)
    const palette = ['#6366f1', '#22c55e', '#f59e0b', '#ef4444', '#ec4899', '#06b6d4', '#8b5cf6'];
    const setupSwatch = (id) => {
        const swatch = document.getElementById(id);
        if (!swatch) return;
        let index = 0;
        swatch.addEventListener('click', () => {
            index = (index + 1) % palette.length;
            swatch.style.backgroundColor = palette[index];
        });
    };
    setupSwatch('swatch-bbox');
    setupSwatch('swatch-snap');

    // Collapsible Groupbox
    const colHeader = document.getElementById('col-header-demo');
    const colBody = document.getElementById('col-body-demo');
    const colArrow = document.getElementById('col-arrow');
    let colExpanded = true;

    if (colHeader && colBody) {
        colHeader.addEventListener('click', () => {
            colExpanded = !colExpanded;
            if (colExpanded) {
                colBody.classList.remove('collapsed');
                if (colArrow) colArrow.style.transform = 'rotate(0deg)';
            } else {
                colBody.classList.add('collapsed');
                if (colArrow) colArrow.style.transform = 'rotate(-90deg)';
            }
        });
    }

    // Interactive Float Slider
    const fovTrack = document.getElementById('fov-track');
    const fovThumb = document.getElementById('fov-thumb');
    const fovFill = document.getElementById('fov-fill');
    const fovVal = document.getElementById('fov-val');

    if (fovTrack && fovThumb && fovFill && fovVal) {
        let draggingFov = false;

        const updateFov = (clientX) => {
            const rect = fovTrack.getBoundingClientRect();
            let ratio = (clientX - rect.left) / rect.width;
            ratio = Math.max(0, Math.min(1, ratio));
            const degrees = (60 + ratio * 60).toFixed(1);
            fovThumb.style.left = `${ratio * 100}%`;
            fovFill.style.width = `${ratio * 100}%`;
            fovVal.textContent = `${degrees}°`;
        };

        fovTrack.addEventListener('mousedown', (e) => {
            draggingFov = true;
            updateFov(e.clientX);
        });

        window.addEventListener('mousemove', (e) => {
            if (draggingFov) updateFov(e.clientX);
        });

        window.addEventListener('mouseup', () => {
            draggingFov = false;
        });
    }

    // Dropdown Box & Search Filter
    const dropdownBox = document.getElementById('dropdown-box');
    const dropdownMenu = document.getElementById('dropdown-menu');
    const dropdownSearch = document.getElementById('dropdown-search');
    const selectedBoneText = document.getElementById('selected-bone-text');

    if (dropdownBox && dropdownMenu) {
        dropdownBox.addEventListener('click', (e) => {
            e.stopPropagation();
            dropdownMenu.classList.toggle('open');
            if (dropdownSearch) dropdownSearch.focus();
        });

        document.querySelectorAll('.dropdown-item').forEach(item => {
            item.addEventListener('click', () => {
                document.querySelectorAll('.dropdown-item').forEach(i => i.classList.remove('selected'));
                item.classList.add('selected');
                if (selectedBoneText) {
                    selectedBoneText.textContent = item.getAttribute('data-value');
                }
                dropdownMenu.classList.remove('open');
            });
        });

        if (dropdownSearch) {
            dropdownSearch.addEventListener('input', (e) => {
                const query = e.target.value.toLowerCase();
                document.querySelectorAll('.dropdown-item').forEach(item => {
                    const text = item.textContent.toLowerCase();
                    item.style.display = text.includes(query) ? 'flex' : 'none';
                });
            });
        }

        window.addEventListener('click', (e) => {
            if (!dropdownMenu.contains(e.target) && e.target !== dropdownBox) {
                dropdownMenu.classList.remove('open');
            }
        });
    }

    // -------------------------------------------------------------------------
    // 3. Smooth Navigation Anchor Scrolling
    // -------------------------------------------------------------------------
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
