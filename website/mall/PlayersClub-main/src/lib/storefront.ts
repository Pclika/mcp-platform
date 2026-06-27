export const mockAddresses = [
  {
    id: "addr_primary",
    label: "Primary Warehouse",
    name: "Pclika Labs",
    street: "88 Innovation Road",
    city: "Shenzhen",
    province: "Guangdong",
    postcode: "518000",
    country: "CN",
    phone: "+86 755 0000 8888",
    default: true,
  },
  {
    id: "addr_us",
    label: "US Receiving",
    name: "Pclika US",
    street: "2211 Mission Street",
    city: "San Francisco",
    province: "CA",
    postcode: "94110",
    country: "US",
    phone: "+1 415 555 0182",
    default: false,
  },
];

export const mockOrders = [
  {
    number: "PCL-2026-1008",
    state: "fulfilled",
    paymentState: "paid",
    shipmentState: "shipped",
    total: "$218.00",
    placedAt: "2026-06-18",
    items: 3,
  },
  {
    number: "PCL-2026-1009",
    state: "cart",
    paymentState: "awaiting_payment",
    shipmentState: "ready",
    total: "$89.00",
    placedAt: "2026-06-24",
    items: 1,
  },
];

export const shippingMethods = [
  {
    id: "dhl_express",
    name: "DHL Express",
    eta: "2-4 business days",
    price: "$28.00",
    description: "Fast cross-border delivery with live tracking.",
  },
  {
    id: "priority_line",
    name: "Priority Line",
    eta: "5-8 business days",
    price: "$14.00",
    description: "Balanced cost and speed for maker and SMB orders.",
  },
  {
    id: "warehouse_pickup",
    name: "Shenzhen Pickup",
    eta: "Same day",
    price: "$0.00",
    description: "For local pickup or supply-chain partner handoff.",
  },
];

export const paymentMethods = [
  {
    id: "stripe_card",
    name: "Credit / Debit Card",
    description: "Stripe-backed card checkout for international cards.",
  },
  {
    id: "paypal",
    name: "PayPal",
    description: "PayPal redirect and buyer protection for global customers.",
  },
  {
    id: "bank_transfer",
    name: "Bank Transfer",
    description: "Manual confirmation for ToB and larger purchase orders.",
  },
];

export const checkoutSummary = {
  items: 3,
  subtotal: "$176.00",
  shipping: "$28.00",
  tax: "$14.00",
  total: "$218.00",
};
