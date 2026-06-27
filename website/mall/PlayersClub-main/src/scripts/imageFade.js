const init = () => {
  // Retrieve all images with the 'fade-in' class.
  const images = document.querySelectorAll('img.fade-in');

  // Set up load handlers to add a 'loaded' class when images finish loading.
  images.forEach((img) => {
    img.onload = function () {
      img.classList.add('loaded'); // Mark the image as loaded.
    };

    // If the image is already cached, trigger the load handler immediately.
    if (img.complete) {
      img.onload();
    }
  });
};

// Initialize fade-in effects on Astro page load.
document.addEventListener('astro:page-load', init);