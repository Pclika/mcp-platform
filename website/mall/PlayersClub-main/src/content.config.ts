import { defineCollection, z } from 'astro:content';
import { glob } from 'astro/loaders';

const products = defineCollection({
  loader: glob({ pattern: '**/*.md', base: "./src/data/products" }),
  schema: z.object({
    name: z.string(),
    sku: z.string(),
    category: z.enum(['Board', 'Sensor', 'Display', 'Driver', 'Bridge', 'Power', 'Module']),
    price: z.string(),
    image: z.object({
      src: z.string(),
      alt: z.string(),
    }),
    images: z.array(z.object({
      src: z.string(),
      alt: z.string(),
    })).optional(),
    specs: z.object({
      chip: z.string().optional(),
      voltage: z.string().optional(),
      protocol: z.string().optional(),
      pins: z.string().optional(),
      dimensions: z.string().optional(),
      weight: z.string().optional(),
      interface: z.string().optional(),
      resolution: z.string().optional(),
      channels: z.string().optional(),
    }),
  }),
});

const policies = defineCollection({
  loader: glob({ pattern: '**/*.md', base: "./src/data/policies" }),
  schema: z.object({
    title: z.string(),
    last_updated: z.string(),
  }),
});

export const collections = { products, policies };
