export const DEFAULT_LOCALE = "en_US";

export const syliusPaths = {
  homepage: (locale = DEFAULT_LOCALE) => `/${locale}/`,
  login: (locale = DEFAULT_LOCALE) => `/${locale}/login`,
  loginCheck: (locale = DEFAULT_LOCALE) => `/${locale}/login-check`,
  register: (locale = DEFAULT_LOCALE) => `/${locale}/register`,
  registerThankYou: (locale = DEFAULT_LOCALE) => `/${locale}/register/thank-you`,
  registerAfterCheckout: (tokenValue: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/register-after-checkout/${tokenValue}`,
  forgotPassword: (locale = DEFAULT_LOCALE) => `/${locale}/forgotten-password`,
  resetPassword: (token: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/forgotten-password/${token}`,
  taxon: (slug: string, locale = DEFAULT_LOCALE) => `/${locale}/taxons/${slug}`,
  product: (slug: string, locale = DEFAULT_LOCALE) => `/${locale}/products/${slug}`,
  productReviews: (slug: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/products/${slug}/reviews/`,
  newProductReview: (slug: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/products/${slug}/reviews/new`,
  cart: (locale = DEFAULT_LOCALE) => `/${locale}/cart/`,
  cartCheckout: (locale = DEFAULT_LOCALE) => `/${locale}/cart/checkout`,
  checkoutStart: (locale = DEFAULT_LOCALE) => `/${locale}/checkout/`,
  checkoutAddress: (locale = DEFAULT_LOCALE) => `/${locale}/checkout/address`,
  checkoutShipping: (locale = DEFAULT_LOCALE) => `/${locale}/checkout/select-shipping`,
  checkoutPayment: (locale = DEFAULT_LOCALE) => `/${locale}/checkout/select-payment`,
  checkoutComplete: (locale = DEFAULT_LOCALE) => `/${locale}/checkout/complete`,
  contact: (locale = DEFAULT_LOCALE) => `/${locale}/contact/`,
  orderThankYou: (locale = DEFAULT_LOCALE) => `/${locale}/order/thank-you`,
  orderShow: (tokenValue: string, locale = DEFAULT_LOCALE) => `/${locale}/order/${tokenValue}`,
  orderPay: (tokenValue: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/order/${tokenValue}/pay`,
  orderAfterPay: (hash: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/order/after-pay/${hash}`,
  accountRoot: (locale = DEFAULT_LOCALE) => `/${locale}/account/`,
  accountDashboard: (locale = DEFAULT_LOCALE) => `/${locale}/account/dashboard`,
  accountOrders: (locale = DEFAULT_LOCALE) => `/${locale}/account/orders/`,
  accountOrderShow: (number: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/account/orders/${number}`,
  accountAddressBook: (locale = DEFAULT_LOCALE) => `/${locale}/account/address-book/`,
  accountAddressBookAdd: (locale = DEFAULT_LOCALE) =>
    `/${locale}/account/address-book/add`,
  accountAddressBookEdit: (id: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/account/address-book/${id}/edit`,
  accountAddressBookSetDefault: (id: string, locale = DEFAULT_LOCALE) =>
    `/${locale}/account/address-book/${id}/set-as-default`,
  accountProfileEdit: (locale = DEFAULT_LOCALE) => `/${locale}/account/profile/edit`,
  accountChangePassword: (locale = DEFAULT_LOCALE) =>
    `/${locale}/account/change-password`,
};

export const storefrontAliases = {
  home: "/",
  login: "/login",
  register: "/register",
  cart: "/cart",
  checkout: "/checkout",
  checkoutAddress: "/checkout/address",
  checkoutShipping: "/checkout/select-shipping",
  checkoutPayment: "/checkout/select-payment",
  checkoutComplete: "/checkout/complete",
  account: "/account",
  accountDashboard: "/account/dashboard",
  accountOrders: "/account/orders",
  accountAddressBook: "/account/address-book",
  accountAddressBookAdd: "/account/address-book/add",
  accountProfile: "/account/profile/edit",
  accountPassword: "/account/change-password",
  forgotPassword: "/forgotten-password",
  contact: "/contact",
  taxons: "/taxons",
  products: "/products",
  orderThankYou: "/order/thank-you",
};

export const syliusCapabilities = [
  "cart",
  "checkout",
  "payment",
  "orders",
  "account",
  "address-book",
  "auth",
  "taxons",
  "reviews",
];
