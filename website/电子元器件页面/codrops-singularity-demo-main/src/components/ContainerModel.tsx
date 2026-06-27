import { useEffect, useRef } from 'react';
import { BallCollider, RigidBody } from '@react-three/rapier';
import * as THREE from 'three';
import DraggableRigidBody from './DraggableRigidBody';

import { Ak47Model } from './models/Ak47';
import { ApeModel } from './models/Ape';
import { BooksModel } from './models/Books';
import { CarModel } from './models/Car';
import { ClankModel } from './models/Clank';
import { ControllerModel } from './models/Controller';
import { CrashModel } from './models/Crash';
import { DaftpunkModel } from './models/Daftpunk';
import { Deadmau5Model } from './models/Deadmau5';
import { DexterModel } from './models/Dexter';
import { VinylsModel } from './models/Vinyls';
import { GbcModel } from './models/Gbc';
import { HaloModel } from './models/Halo';
import { KanyeModel } from './models/Kanye';
import { KlonoaModel } from './models/Klonoa';
import { MarioModel } from './models/Mario';
import { MinecraftModel } from './models/Minecraft';
import { RaymanModel } from './models/Rayman';
import { SlyModel } from './models/Sly';
import { SnakeModel } from './models/Snake';
import { SteveModel } from './models/Steve';
import { TankModel } from './models/Tank';
import { VhsModel } from './models/Vhs';
import { ZeldaModel } from './models/Zelda';
import { ZombieModel } from './models/Zombie';
import { CdsModel } from './models/Cds';
import { DvdsModel } from './models/Dvds';
import TexturedPlane from './models/PngMesh';
import { useFrame, useThree } from '@react-three/fiber';

// Array of generated positions
export const generatedPositions = [
  new THREE.Vector3(-1.95, 5.98, 7.87),
  new THREE.Vector3(0.14, 2.83, 6.97),
  new THREE.Vector3(-3.30, 17.29, -7.24),
  new THREE.Vector3(1.76, 8.91, 7.82),
  new THREE.Vector3(-1.15, 5.78, 2.83),
  new THREE.Vector3(-3.73, 18.46, 6.17),
  new THREE.Vector3(-8.70, 7.96, -3.83),
  new THREE.Vector3(2.82, 11.15, 7.21),
  new THREE.Vector3(8.10, 2.48, -6.10),
  new THREE.Vector3(-3.15, 4.90, 7.86),
  new THREE.Vector3(0.12, 14.69, -8.20),
  new THREE.Vector3(-4.83, 5.86, -3.71),
  new THREE.Vector3(2.84, 11.53, -6.39),
  new THREE.Vector3(0.18, 6.47, -1.55),
  new THREE.Vector3(7.89, 3.43, 3.89),
  new THREE.Vector3(-4.98, 9.38, -3.18),
  new THREE.Vector3(5.85, 12.10, 3.41),
  new THREE.Vector3(-4.07, 8.77, -0.31),
  new THREE.Vector3(-6.32, 14.35, -4.94),
  new THREE.Vector3(-5.39, 9.99, -1.62),
  new THREE.Vector3(-0.75, 3.73, 7.48),
  new THREE.Vector3(-4.27, 11.08, -5.92),
  new THREE.Vector3(8.99, 10.35, -6.74),
  new THREE.Vector3(8.64, 1.15, 2.86),
  new THREE.Vector3(-1.78, 18.51, -1.10),
  new THREE.Vector3(-7.37, 5.75, -5.19),
  new THREE.Vector3(5.65, 16.12, -1.13),
  new THREE.Vector3(-4.48, 3.52, 4.49),
  new THREE.Vector3(2.68, 18.87, -4.03),
  new THREE.Vector3(-4.20, 16.40, 1.62),
  new THREE.Vector3(-0.64, 1.85, 2.51),
  new THREE.Vector3(2.29, 10.98, -7.90),
  new THREE.Vector3(-3.94, 1.80, 6.38),
  new THREE.Vector3(5.28, 5.28, -4.67),
  new THREE.Vector3(-8.99, 10.95, -8.96),
  new THREE.Vector3(-6.12, 7.13, 8.60),
  new THREE.Vector3(5.78, 15.10, -4.82),
  new THREE.Vector3(2.72, 7.64, -7.12),
  new THREE.Vector3(5.30, 4.67, 1.91),
  new THREE.Vector3(-4.78, 8.88, -6.19),
  new THREE.Vector3(8.98, 6.70, -4.93),
  new THREE.Vector3(2.94, 18.38, -5.00),
  new THREE.Vector3(-8.62, 10.55, -2.34),
  new THREE.Vector3(8.97, 9.11, 6.33),
  new THREE.Vector3(-5.86, 5.15, -4.94),
  new THREE.Vector3(-5.21, 5.23, -1.77),
  new THREE.Vector3(-0.45, 11.63, -7.62),
  new THREE.Vector3(-4.02, 18.47, -6.95),
  new THREE.Vector3(7.39, 3.87, 5.14),
  new THREE.Vector3(-8.49, 14.76, 4.70),
  new THREE.Vector3(6.13, 2.07, -6.64),
  new THREE.Vector3(2.64, 14.01, 5.43),
  new THREE.Vector3(2.60, 5.10, -0.29),
  new THREE.Vector3(2.66, 14.20, -8.76),
  new THREE.Vector3(-0.71, 18.23, -2.29),
  new THREE.Vector3(-4.43, 11.18, 6.92),
  new THREE.Vector3(6.59, 16.54, 6.05),
  new THREE.Vector3(1.16, 4.72, 8.34),
  new THREE.Vector3(8.11, 9.82, 2.62),
  new THREE.Vector3(-8.14, 1.51, -4.14),
  new THREE.Vector3(-3.16, 7.15, -2.00),
  new THREE.Vector3(-4.33, 9.05, 2.49),
  new THREE.Vector3(8.78, 8.17, -8.17),
  new THREE.Vector3(-0.85, 6.01, 7.81),
  new THREE.Vector3(1.77, 4.06, 6.51),
  new THREE.Vector3(4.92, 12.70, -6.28),
  new THREE.Vector3(-8.70, 15.85, 2.95),
  new THREE.Vector3(-6.13, 17.78, 4.43),
  new THREE.Vector3(-2.71, 10.80, 2.52),
  new THREE.Vector3(-8.51, 7.09, 1.89),
  new THREE.Vector3(-6.53, 5.82, 3.28),
  new THREE.Vector3(0.20, 18.83, -4.17),
  new THREE.Vector3(0.08, 18.19, -8.47),
  new THREE.Vector3(3.34, 2.72, 5.62),
  new THREE.Vector3(3.86, 16.34, -8.75),
  new THREE.Vector3(-5.30, 17.13, -4.59),
  new THREE.Vector3(6.01, 11.09, 4.13),
  new THREE.Vector3(2.51, 14.94, 0.22),
  new THREE.Vector3(4.21, 14.53, 0.28),
  new THREE.Vector3(-7.74, 3.12, 1.07),
  new THREE.Vector3(-3.57, 7.92, -5.89),
  new THREE.Vector3(4.71, 15.00, 1.48),
  new THREE.Vector3(1.04, 10.51, 0.71),
  new THREE.Vector3(7.20, 8.89, 0.73),
  new THREE.Vector3(-0.90, 17.70, 6.40),
  new THREE.Vector3(6.87, 16.43, 2.50),
  new THREE.Vector3(-4.96, 16.04, 7.64),
  new THREE.Vector3(-6.41, 9.58, 7.38),
  new THREE.Vector3(8.05, 17.96, -3.81),
  new THREE.Vector3(-1.44, 16.83, 0.44),
  new THREE.Vector3(-1.35, 2.40, -7.51),
  new THREE.Vector3(3.65, 14.03, 3.10),
  new THREE.Vector3(8.50, 12.47, 4.30),
  new THREE.Vector3(7.15, 15.92, -0.59),
  new THREE.Vector3(-5.82, 6.86, 1.91),
  new THREE.Vector3(0.17, 4.77, 2.82),
  new THREE.Vector3(-5.78, 18.06, -5.03),
  new THREE.Vector3(-1.59, 14.86, -0.05),
  new THREE.Vector3(-0.02, 9.68, -8.17),
  new THREE.Vector3(2.43, 12.36, -8.32)
];

function Pointer({ planeNormal = new THREE.Vector3(0, 0, 1), planeConstant = 0 }) {
  const ref = useRef<any>(undefined)
  const { camera } = useThree()
  const raycaster = new THREE.Raycaster()
  const plane = new THREE.Plane(planeNormal, planeConstant)
  const targetPosition = useRef(new THREE.Vector3());

  const updateTargetPosition = (event: MouseEvent | TouchEvent) => {
    const mouse = event instanceof MouseEvent ?
      new THREE.Vector2((event.clientX / window.innerWidth) * 2 - 1, -(event.clientY / window.innerHeight) * 2 + 1) :
      new THREE.Vector2((event.touches[0].clientX / window.innerWidth) * 2 - 1, -(event.touches[0].clientY / window.innerHeight) * 2 + 1);

    raycaster.setFromCamera(mouse, camera);
    raycaster.ray.intersectPlane(plane, targetPosition.current);
  };

  useEffect(() => {
    window.addEventListener('mousemove', updateTargetPosition);
    window.addEventListener('touchmove', updateTargetPosition);

    return () => {
      window.removeEventListener('mousemove', updateTargetPosition);
      window.removeEventListener('touchmove', updateTargetPosition);
    };
  }, []);

  useFrame(() => {
    ref.current?.setNextKinematicTranslation(targetPosition.current);
  });

  return (
    <RigidBody position={[0, 10, 0]} type="kinematicPosition" colliders={false} ref={ref}>
      <BallCollider args={[1]} />
    </RigidBody>
  )
}

export function ContainerModel(props: JSX.IntrinsicElements['group'] & { enablePointer?: boolean, lowSetting?: boolean }) {

  const springJoint = true;
  const objectScaleSize = 2.5;
  const imageArray = Array.from({ length: 11 }, (v, i) => `${i + 1}-min.jpg`);
  const vhsArray = Array.from({ length: 5 }, (v, i) => `${i + 1}.jpg`);
  const cdsArray = Array.from({ length: 33 }, (v, i) => `${i + 1}-min.jpg`);
  const vinylsArray = Array.from({ length: 9 }, (v, i) => `${i + 1}.jpg`);
  const booksArray = Array.from({ length: 7 }, (v, i) => `${i + 1}-min.jpg`);
  const logosArray = Array.from({ length: 12 }, (v, i) => `${i + 1}.png`);

  const DraggableRigidBodyProps: Partial<any> = {
    rigidBodyProps: {
      restitution: 0,
      angularDamping: .2,
    },
    jointConfig: {
      restLength: .01,
      stiffness: 3500,
    },
    boundingBox: [[-40, 40], [0.01, 50], [-40, 40]],
    dragControlsProps: {
      preventOverlap: true
    },
    enableSpringJoint: springJoint,
  }

  return (
    <>
      {props.enablePointer && <Pointer />}

      {!props.lowSetting && (
        <CdsModel
          texturesSrc={cdsArray.map(p => `/cds/${p}`)}
          groupProps={{}}
          draggableRigidBodyProps={DraggableRigidBodyProps}
        />
      )}

      {!props.lowSetting && (
        <VinylsModel
          texturesSrc={vinylsArray.map(p => `/vinyls/${p}`)}
          groupProps={{}}
          draggableRigidBodyProps={DraggableRigidBodyProps}
        />
      )}

      {!props.lowSetting && (
        <DvdsModel
          texturesSrc={imageArray.map(p => `/films/${p}`)}
          groupProps={{}}
          draggableRigidBodyProps={DraggableRigidBodyProps}
        />
      )}

      {!props.lowSetting && (
        <VhsModel
          texturesSrc={vhsArray.map(p => `/vhs/${p}`)}
          groupProps={{}}
          draggableRigidBodyProps={DraggableRigidBodyProps}
        />
      )}

      {!props.lowSetting && (
        <BooksModel
          texturesSrc={booksArray.map(p => `/comics/${p}`)}
          groupProps={{}}
          draggableRigidBodyProps={DraggableRigidBodyProps}
        />
      )}

      {logosArray.map((p, i) => {
        return <DraggableRigidBody  {...DraggableRigidBodyProps}
          key={`logo${i}`}
          groupProps={{
            position: generatedPositions[9 + 7 + 5 + 33 + 11 + i]
          }}
          enableSpringJoint={springJoint}
          rigidBodyProps={{ colliders: 'cuboid', density: 20, angularDamping: .1, linearDamping: .1 }}
          visibleMesh={
            <TexturedPlane src={`/logos/${p}`} />
          }
        />
      })}

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 0] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'ball' }}
        visibleMesh={
          <MarioModel position={[0, 0, 0]} scale={objectScaleSize * .14} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 1] }}
        enableSpringJoint={springJoint}
        visibleMesh={
          <ZombieModel scale={objectScaleSize * .6} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 2] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <Ak47Model scale={objectScaleSize * 0.04} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 3] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'ball' }}
        visibleMesh={
          <ApeModel scale={objectScaleSize * .8} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 5] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <CarModel scale={objectScaleSize * .1} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 6] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'ball' }}
        visibleMesh={
          <ClankModel scale={objectScaleSize * .7} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 7] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <ControllerModel scale={objectScaleSize * .005} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 8] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <CrashModel scale={objectScaleSize * .01} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 9] }}
        enableSpringJoint={springJoint}
        visibleMesh={
          <DaftpunkModel scale={objectScaleSize * 3.7} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 10] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <Deadmau5Model scale={objectScaleSize * .05} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 11] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <DexterModel scale={objectScaleSize * .1} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 13] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ density: 4 }}
        visibleMesh={
          <GbcModel scale={objectScaleSize * .7} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 14] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid', density: 10, angularDamping: .5 }}
        visibleMesh={
          <HaloModel scale={objectScaleSize * 1} />
        }
      />
      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 15] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'ball' }}
        visibleMesh={
          <KanyeModel scale={objectScaleSize * 2} />
        }
      />
      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 16] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid', density: 7 }}
        visibleMesh={
          <KlonoaModel scale={objectScaleSize * .27} />
        }
      />
      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 17] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <MinecraftModel scale={objectScaleSize * 1.8} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 19] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'ball' }}
        visibleMesh={
          <RaymanModel scale={objectScaleSize * 1.3} />
        }
      />
      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 20] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid', density: 10, angularDamping: .5 }}
        visibleMesh={
          <SlyModel scale={objectScaleSize * 1} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 21] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <SnakeModel scale={objectScaleSize * .13} />
        }
      />
      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 22] }}
        enableSpringJoint={springJoint}
        visibleMesh={
          <SteveModel scale={objectScaleSize * .06} />
        }
      />
      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 23] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <TankModel scale={objectScaleSize * .8} />
        }
      />

      <DraggableRigidBody  {...DraggableRigidBodyProps}
        groupProps={{ position: generatedPositions[78 + 26] }}
        enableSpringJoint={springJoint}
        rigidBodyProps={{ colliders: 'cuboid' }}
        visibleMesh={
          <ZeldaModel scale={objectScaleSize * .05} />
        }
      />

    </>
  );
}