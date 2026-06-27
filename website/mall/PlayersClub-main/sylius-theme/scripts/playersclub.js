/**
 * Players Club theme — JavaScript animation modules
 * for Sylius 2.x ShopBundle.
 *
 * Modules:
 *   - initImageFade     — IntersectionObserver for .pc-fade-in images
 *   - initTooltips      — mouse-follow tooltip on .pc-product-card--detail
 *   - initTextSliders   — hover text-swap animation
 *   - initSmoothScroll  — CSS scroll-behavior polyfill
 *   - initSearch        — search dialog toggle + grid filter
 */
(function () {
  'use strict';

  /* ── Image Fade-in ── */
  function initImageFade() {
    var observer = new IntersectionObserver(
      function (entries) {
        entries.forEach(function (entry) {
          if (entry.isIntersecting) {
            entry.target.classList.add('pc-loaded');
            observer.unobserve(entry.target);
          }
        });
      },
      { threshold: 0.1 }
    );

    document.querySelectorAll('.pc-fade-in').forEach(function (el) {
      observer.observe(el);
    });
  }

  /* ── Tooltips ── */
  function initTooltips() {
    var tooltip = document.createElement('div');
    tooltip.className = 'pc-tooltip';
    tooltip.innerHTML =
      '<div class="pc-tooltip__row" data-field="name">' +
      '<div class="pc-tooltip__content"><span class="pc-textslide"><span class="pc-textslide__inner"></span></span></div>' +
      '<div class="pc-tooltip__content"><span class="pc-textslide"><span class="pc-textslide__inner"></span></span></div>' +
      '</div>' +
      '<div class="pc-tooltip__row pc-tooltip__row--ra">' +
      '<span class="pc-textslide"><span class="pc-textslide__inner">→</span></span>' +
      '</div>' +
      '<div class="pc-tooltip__row" data-field="sku">' +
      '<div class="pc-tooltip__content"><span class="pc-textslide"><span class="pc-textslide__inner"></span></span></div>' +
      '<div class="pc-tooltip__content"><span class="pc-textslide"><span class="pc-textslide__inner"></span></span></div>' +
      '</div>' +
      '<div class="pc-tooltip__row pc-tooltip__row--ra" data-field="price">' +
      '<div class="pc-tooltip__content"><span class="pc-textslide"><span class="pc-textslide__inner"></span></span></div>' +
      '<div class="pc-tooltip__content"><span class="pc-textslide"><span class="pc-textslide__inner"></span></span></div>' +
      '</div>';

    document.body.appendChild(tooltip);

    var activeCard = null;
    var rafId = null;
    var targetX = 0;
    var targetY = 0;
    var currentX = 0;
    var currentY = 0;

    function lerp(a, b, n) { return (1 - n) * a + n * b; }

    function animate() {
      currentX = lerp(currentX, targetX, 0.15);
      currentY = lerp(currentY, targetY, 0.15);
      tooltip.style.left = currentX + 'px';
      tooltip.style.top = currentY + 'px';
      rafId = requestAnimationFrame(animate);
    }

    function setTooltipContent(card) {
      var name = card.getAttribute('data-product-name') || '';
      var sku = card.getAttribute('data-product-sku') || '';
      var price = card.getAttribute('data-product-price') || '';

      var rows = tooltip.querySelectorAll('.pc-tooltip__row');
      rows.forEach(function (row) {
        var field = row.getAttribute('data-field');
        var inners = row.querySelectorAll('.pc-textslide__inner');
        if (field === 'name') {
          inners.forEach(function (el) { el.textContent = name; });
        } else if (field === 'sku') {
          inners.forEach(function (el) { el.textContent = sku; });
        } else if (field === 'price') {
          inners.forEach(function (el) { el.textContent = price; });
        }
      });
    }

    function showTooltip(card, x, y) {
      if (activeCard !== card) {
        setTooltipContent(card);
        tooltip.style.opacity = '1';
        activeCard = card;
      }
      targetX = x + 20;
      targetY = y + 20;
    }

    function hideTooltip() {
      tooltip.style.opacity = '0';
      activeCard = null;
    }

    // Start animation loop
    animate();

    document.addEventListener('mouseover', function (e) {
      var card = e.target.closest('.pc-product-card--detail');
      if (card) {
        showTooltip(card, e.clientX, e.clientY);
      }
    });

    document.addEventListener('mousemove', function (e) {
      if (activeCard) {
        targetX = e.clientX + 20;
        targetY = e.clientY + 20;
      }
    });

    document.addEventListener('mouseout', function (e) {
      if (activeCard && !e.target.closest('.pc-product-card--detail')) {
        hideTooltip();
      }
    });
  }

  /* ── Text Sliders ── */
  function initTextSliders() {
    // Simple hover effect: translateY on inner span
    document.querySelectorAll('.pc-textslide').forEach(function (el) {
      var inner = el.querySelector('.pc-textslide__inner');
      if (!inner) return;

      var originalText = inner.textContent;
      var transitioning = false;

      el.addEventListener('mouseenter', function () {
        if (transitioning) return;
        transitioning = true;
        inner.style.transition = 'transform 0.2s ease';
        inner.style.transform = 'translateY(-105%)';
        setTimeout(function () {
          inner.style.transition = 'none';
          inner.style.transform = 'translateY(105%)';
          inner.textContent = originalText;
          // Force reflow
          inner.offsetHeight;
          inner.style.transition = 'transform 0.2s ease';
          inner.style.transform = 'translateY(0)';
          setTimeout(function () {
            transitioning = false;
          }, 200);
        }, 200);
      });
    });
  }

  /* ── Smooth Scroll ── */
  function initSmoothScroll() {
    // Use native CSS smooth scrolling
    document.documentElement.style.scrollBehavior = 'smooth';

    // Smooth scroll for anchor links
    document.addEventListener('click', function (e) {
      var link = e.target.closest('a[href^="#"]');
      if (!link) return;

      var targetId = link.getAttribute('href');
      if (targetId === '#') return;

      var target = document.querySelector(targetId);
      if (target) {
        e.preventDefault();
        target.scrollIntoView({ behavior: 'smooth', block: 'start' });
      }
    });
  }

  /* ── Search / Grid Filter ── */
  function initSearch() {
    var searchInput = document.getElementById('pc-search-input');
    if (!searchInput) return;

    var grid = document.getElementById('pc-product-grid');
    if (!grid) return;

    searchInput.addEventListener('input', function () {
      var query = this.value.toLowerCase().trim();
      var cards = grid.querySelectorAll('.pc-product-card--detail');

      cards.forEach(function (card) {
        var name = (card.getAttribute('data-product-name') || '').toLowerCase();
        var sku = (card.getAttribute('data-product-sku') || '').toLowerCase();

        if (!query || name.indexOf(query) !== -1 || sku.indexOf(query) !== -1) {
          card.style.display = '';
          card.parentElement.style.display = '';
        } else {
          card.style.display = 'none';
          card.parentElement.style.display = 'none';
        }
      });
    });
  }

  /* ── Init all on DOM ready ── */
  document.addEventListener('DOMContentLoaded', function () {
    // Only run within the Players Club theme
    if (!document.querySelector('.playersclub-theme')) return;

    initImageFade();
    initTooltips();
    initTextSliders();
    initSmoothScroll();
    initSearch();
  });
})();
