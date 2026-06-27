import '../styles/main.scss'

import ProductGrid from './product-grid'
import { preloadImages } from './utils'

window.addEventListener('load', async () => {
  new ProductGrid()

  // load all the images
  const images = [...document.querySelectorAll('img')]
  await preloadImages(images).then(() => {
    document.body.classList.remove('loading')
  })
})
