import { Physics, CuboidCollider } from "@react-three/rapier";
import { Environment, PerformanceMonitor, Shadow } from '@react-three/drei';
import { ContainerModel } from './ContainerModel';
import { Canvas } from '@react-three/fiber';
import { Suspense, useEffect, useState } from 'react';
import { Attractor } from "@react-three/rapier-addons";
import { button, folder, Leva, useControls } from 'leva';
import { BlendFunction } from 'postprocessing';
import * as THREE from "three";
import {
  EffectComposer,
  N8AO,
  Noise,
  HueSaturation,
  ToneMapping,
  Bloom,
  DepthOfField,
} from '@react-three/postprocessing';
import { Perf } from 'r3f-perf';
import { GeistSans } from 'geist/font/sans';
import { Branding } from '@/components/Branding';
import { SmoothOrbitControls } from '@/components/SmoothOrbitControls';
import { getDevicePixelRatio } from './utils/utils';

export const ContainerScene = () => {
  const [gravity, setGravity] = useState(0);
  const [force, setForce] = useState(3);
  const [attractorEnabled, setAttractorEnabled] = useState(true);
  const [dpr, setDpr] = useState(getDevicePixelRatio);
  const [isAOEnabled, setIsAOEnabled] = useState(true);
  const [isBloomEnabled, setIsBloomEnabled] = useState(true);
  const [lowSetting, setLowSetting] = useState(false);
  const [showPerf, setShowPerf] = useState(false);
  const [showLeva, setShowLeva] = useState(false);
  const [showBranding, setShowBranding] = useState(false);
  const [frameloop, setFrameloop] = useState<'always' | 'never'>('always');

  // prevents performance monitor triggering on tab swap
  useEffect(() => {
    const handleVisibilityChange = () => {
      console.log('change', document.hidden);
      setFrameloop(document.hidden ? 'never' : 'always')
    };

    document.addEventListener('visibilitychange', handleVisibilityChange);

    return () => {
      document.removeEventListener('visibilitychange', handleVisibilityChange);
    };
  }, []);

  const { backgroundEnabled, unlockCamera } = useControls({
    General: folder({
      "Hide settings": button(() => setShowLeva(false)),
      showPerfMonitor: { value: false, onChange: (val) => setShowPerf(val) },
      backgroundEnabled: { value: true },
      unlockCamera: { value: false }
    }, { collapsed: true })
  });

  const physics = useControls({
    physics: folder({
      debugMode: { value: false },
      paused: { value: false },
      gravity: { value: gravity, min: -20, max: 20, step: 0.1, onChange: setGravity },
      attractorForce: { value: force, min: -50, max: 50, onChange: setForce },
      enableMouseRepulsion: { value: true }
    }, { collapsed: true }),
  });

  const bloom = useControls({
    bloom: folder({
      enabled: true,
      radius: { value: .92, min: 0, max: 2, step: .01 },
      intensity: { value: .79, min: 0, max: 10, step: .01 },
      luminanceThreshold: { value: .56, min: 0, max: 10, step: .01 },
      luminanceSmoothing: { value: .55, min: 0, max: 10, step: .01 },
    }, { collapsed: true }),
  });

  const depthOfField = useControls({
    depthOfField: folder({
      enabled: { value: true },
      worldFocusDistance: { value: 30, step: 0.01 },
      bokehScale: { value: 1, min: 0, max: 2, step: 0.001 },
      focusRange: { value: 1.69, min: 0, max: 3, step: 0.001 },
    }, { collapsed: true })
  });

  const hueSaturation = useControls({
    hueSaturation: folder({
      enabled: true,
      saturation: { value: .26, step: .01 },
    }, { collapsed: true })
  });

  useEffect(() => {
    // prevents scroll on mobile
    document.addEventListener('touchmove', preventScroll, { passive: false });

    return () => {
      document.removeEventListener('touchmove', preventScroll);
    }
  }, []);

  const preventScroll = (e: TouchEvent) => {
    e.preventDefault();
  };

  return (
    <>
      <Leva hidden={!showLeva} />

      {showBranding && (
        <>
          <Branding onShowSettings={() => setShowLeva(!showLeva)} />
          <div className='flex justify-center z-50 absolute bottom-[5vh] w-[100%]'>
            <button
              className={`${GeistSans.className} bg-white/60 px-3 py-1 text-sm font-medium text-zinc-800 shadow-lg ring-1 ring-zinc-900/5  rounded-full`} onClick={() => {
                setAttractorEnabled(!attractorEnabled);
                setGravity(attractorEnabled ? -(9.81 * 2) : 0);
              }}>
              {attractorEnabled ? 'DEACTIVATE' : 'ACTIVATE'}
            </button>
          </div>
        </>
      )}

      <Canvas flat className='select-none' color='#2e2e2e' dpr={dpr} frameloop={frameloop}
        camera={{
          near: .1,
          far: 150,
          zoom: 1.55
        }}
        gl={{
          powerPreference: "high-performance",
          alpha: false,
          antialias: false,
          stencil: false,
          depth: false,
          outputColorSpace: THREE.SRGBColorSpace,
        }}
        onCreated={state => {
          setShowBranding(true)
          state.camera.position.y = 20;
          state.camera.position.z = 30;
          state.camera.lookAt(0, 10, 0);
          state.camera.updateProjectionMatrix()
        }}>

        {showPerf && <Perf position='bottom-right' />}

        {unlockCamera && <SmoothOrbitControls minPolarAngle={0} maxPolarAngle={Math.PI / 2} position={[0, 20, 30]} target={[0, 10, 0]} />}

        <PerformanceMonitor
          flipflops={1}
          onFallback={() => setLowSetting(true)}
          iterations={5}
          ms={100}
          threshold={.5}
          bounds={() => [30, 500]}
          onDecline={() => {
            setDpr(dpr * 0.8);
            setIsAOEnabled(false);
            setIsBloomEnabled(false);
          }}
        />

        <Environment
          files="https://dl.polyhaven.org/file/ph-assets/HDRIs/hdr/1k/hanger_exterior_cloudy_1k.hdr"
          ground={backgroundEnabled ? { height: 50, radius: 150, scale: 50 } : undefined}
          background={backgroundEnabled}
        />

        <Suspense fallback={null}>

          <Physics
            timeStep={1 / 30}
            debug={physics.debugMode} gravity={[0, gravity, 0]} paused={physics.paused}>
            {attractorEnabled && <Attractor position={[0, 10, 0]} range={80} strength={force} />}

            <ContainerModel position={[0, 0, -.2]} enablePointer={attractorEnabled && physics.enableMouseRepulsion} lowSetting={lowSetting} />

            <CuboidCollider position={[0, -3, 0]} args={[1000, 3, 1000]} />
            <CuboidCollider position={[0, 43, 0]} args={[1000, 3, 1000]} />
            <CuboidCollider position={[-40, 20, 0]} args={[100, 3, 100]} rotation={[0, 0, Math.PI / 2]} />
            <CuboidCollider position={[43, 20, 0]} args={[100, 3, 100]} rotation={[0, 0, Math.PI / 2]} />
            <CuboidCollider position={[0, 20, 43]} args={[100, 3, 100]} rotation={[Math.PI / 2, 0, 0]} />
            <CuboidCollider position={[0, 20, -43]} args={[100, 3, 100]} rotation={[Math.PI / 2, 0, 0]} />

          </Physics>
        </Suspense>

        <Shadow rotation={[-Math.PI / 2, 0, 0]} scale={40} position={[0, 0, 0]} color="black" opacity={.8} />

        <EffectComposer>
          <ToneMapping />
          {isAOEnabled ? <N8AO quality={'medium'} distanceFalloff={1} aoRadius={1} intensity={4} /> : <></>}
          {hueSaturation.enabled ? <HueSaturation saturation={hueSaturation.saturation} /> : <></>}
          {bloom.enabled && isBloomEnabled ? <Bloom mipmapBlur {...bloom} /> : <></>}
          {/* {tiltShift.enabled ? <TiltShift2 {...tiltShift} /> : <></>} */}
          {depthOfField.enabled ? <DepthOfField {...depthOfField} resolutionScale={.6} /> : <></>}
          <Noise opacity={0.1} premultiply blendFunction={BlendFunction.ADD} />
        </EffectComposer>

      </Canvas >
    </>
  );
}