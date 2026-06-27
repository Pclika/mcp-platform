import ProductPreview from './product-preview'
import debounce from 'debounce'

export default class ProductGrid {
  constructor() {
    this.ui = {
      products: Array.from(document.querySelectorAll('.product')),
      containerLeft: document.querySelector('.product-preview.--left'),
      containerRight: document.querySelector('.product-preview.--right')
    }

    this.productPreviewLeft = null
    this.productPreviewRight = null
    this.activeProduct = null
    this.hoverDelay = null

    this.init()
  }

  init() {
    this.productPreviewLeft = new ProductPreview({
      container: this.ui.containerRight,
      products: this.ui.products.filter((_, i) => i % 4 === 2 || i % 4 === 3)
    })

    this.productPreviewRight = new ProductPreview({
      container: this.ui.containerLeft,
      products: this.ui.products.filter((_, i) => i % 4 === 0 || i % 4 === 1)
    })

    this.addEvents()
  }

  addEvents() {
    window.addEventListener(
      'resize',
      debounce(() => {
        this.productPreviewRight.onResize()
        this.productPreviewLeft.onResize()
      })
    )

    this.ui.products.forEach((product) => {
      const previewController = this.getProductSide(product)

      product.addEventListener('mouseenter', () => this.productMouseEnter(product, previewController))
      product.addEventListener('mouseleave', () => this.productMouseLeave())
    })
  }

  getProductSide = (product) => {
    const i = product.dataset.index
    const isLeft = i % 4 === 0 || i % 4 === 1

    return isLeft ? this.productPreviewLeft : this.productPreviewRight
  }

  productMouseEnter(product, preview) {
    // If another timer (aka hover) was running, cancel it
    if (this.hoverDelay) {
      clearTimeout(this.hoverDelay)
      this.hoverDelay = null
    }

    // Start a new timer
    this.hoverDelay = setTimeout(() => {
      this.activeProduct = product
      preview.setProduct(product)
      this.hoverDelay = null // clear reference
    }, 100)
  }

  productMouseLeave() {
    // If user leaves before debounce completes
    if (this.hoverDelay) {
      clearTimeout(this.hoverDelay)
      this.hoverDelay = null
    }

    if (this.activeProduct) {
      const preview = this.getProductSide(this.activeProduct)
      preview.setProduct(null)
      this.activeProduct = null
    }
  }
}
