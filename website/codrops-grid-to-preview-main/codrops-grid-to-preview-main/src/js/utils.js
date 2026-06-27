export function preloadImages(imageElements) {
  const promises = [...imageElements].map((img) => {
    if (img.complete) return Promise.resolve() // Already cached

    return new Promise((resolve, reject) => {
      img.onload = resolve
      img.onerror = resolve // Resolve even on error to avoid blocking
    })
  })

  return Promise.all(promises)
}
