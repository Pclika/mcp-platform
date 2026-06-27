import DraggableRigidBody, { DraggableRigidBodyProps } from '../DraggableRigidBody';
import { GroupProps, useFrame, useLoader } from '@react-three/fiber';
import { useGLTF, Instances, Instance } from '@react-three/drei';
import React, { useMemo, useRef } from 'react';
import { generatedPositions } from '../ContainerModel';
import { GLTF } from 'three-stdlib';
import * as THREE from 'three';

type GLTFResult = GLTF & {
  nodes: {
    Plane: THREE.Mesh;
  };
  materials: {
    ['Material.001']: THREE.MeshStandardMaterial;
  };
  animations: any[];
};

type BookModelProps = JSX.IntrinsicElements['group'] & {
  texturesSrc: string[];
  draggableRigidBodyProps: Partial<DraggableRigidBodyProps>;
  groupProps: GroupProps;
};

export const BooksModel = React.forwardRef<any, BookModelProps>(({ texturesSrc, draggableRigidBodyProps, groupProps, ...props }, ref) => {
  const { nodes, materials } = useGLTF('/models-transformed/book-transformed.glb') as GLTFResult;
  const textures = useLoader(THREE.TextureLoader, texturesSrc);
  const instances = useRef<THREE.InstancedMesh>(null);
  const meshRefs = useMemo(() => texturesSrc.map(() => React.createRef<THREE.Group>()), [texturesSrc]);
  const geometry = useMemo(() => new THREE.PlaneGeometry(0.666, 1), []);
  const geometry2 = useMemo(() => new THREE.BoxGeometry(0.666, 1, 0.15), []);
  const material = useMemo(() => new THREE.MeshStandardMaterial({ visible: false }), []);

  useFrame(() => {
    if (!instances.current) return;

    instances.current.children
      .filter(instance => !!(instance as any).instance)
      .forEach((instance, i) => {
        const p = new THREE.Vector3();
        const r = new THREE.Quaternion();

        if (meshRefs[i]?.current) {
          meshRefs[i].current?.getWorldPosition(p);
          meshRefs[i].current?.getWorldQuaternion(r);
        }

        instance.setRotationFromQuaternion(r);
        instance.rotateX(Math.PI / 2);
        instance.position.set(p.x, p.y, p.z);
      });
  });

  return (
    <Instances ref={instances as any}>
      {/* Define instanced geometry (merged) and material */}
      <bufferGeometry {...nodes.Plane.geometry} />
      <meshStandardMaterial {...materials['Material.001']} />

      {textures.map((texture: THREE.Texture, i:number) => (
        <React.Fragment key={`fragment-${i}`}>
          <Instance scale={[1, 1, 1.33]} rotation={[Math.PI / 2, 0, 0]} />

          <DraggableRigidBody
            key={`cd${i}`}
            {...draggableRigidBodyProps}
            groupProps={{ position: generatedPositions[9 + i] }}
            rigidBodyProps={{ colliders: 'cuboid', density: 3 }}
            enableSpringJoint={true}
            visibleComponentRef={meshRefs[i]}
            visibleMesh={
              <group ref={ref} {...props} dispose={null} rotation={[0, Math.PI, 0]}>
                <mesh scale={2.64} position={[.08, 0, -.09]} rotation={[0, 0.006, 0]} material={material} geometry={geometry2} />

                <mesh scale={2.64} position={[.08, 0, -.09]} rotation={[0, 0.006, 0]} geometry={geometry}>
                  <meshBasicMaterial map={texture} side={THREE.DoubleSide} />
                </mesh>
              </group>
            }
          />
        </React.Fragment>
      ))}
    </Instances>
  );
});

useGLTF.preload('/models-transformed/book-transformed.glb');