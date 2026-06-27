"use client";

import { ContainerScene } from '@/components/CustomScene';
import dynamic from 'next/dynamic';
const DynamicLoadingScreen = dynamic(() => import('../components/LoadingScreen'), { ssr: false })


export default function Home() {
  return (
    <div style={{ height: 'calc(100dvh)' }} className='bg-[#2e2e2e] w-full'>
      <DynamicLoadingScreen />
      <ContainerScene />
    </div>
  );
}
