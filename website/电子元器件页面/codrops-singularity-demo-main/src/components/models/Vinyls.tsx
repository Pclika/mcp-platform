import * as THREE from 'three';
import React, { useMemo, useRef } from 'react';
import { useGLTF, Instances, Instance } from '@react-three/drei';
import { GroupProps, useFrame, useLoader } from '@react-three/fiber';
import DraggableRigidBody, { DraggableRigidBodyProps } from '../DraggableRigidBody';
import { generatedPositions } from '../ContainerModel';
import { GLTF } from 'three-stdlib';

type GLTFResult = GLTF & {
  nodes: {
    vinile: THREE.Mesh
  }
  materials: {
    Vinyl1: THREE.MeshStandardMaterial
  }
}

type DiscModelProps = JSX.IntrinsicElements['group'] & {
  texturesSrc: string[];
  draggableRigidBodyProps: Partial<DraggableRigidBodyProps>;
  groupProps: GroupProps;
};

export const VinylsModel = React.memo(React.forwardRef<any, DiscModelProps>(({ texturesSrc, ...props }, ref) => {
  const { nodes } = useGLTF('/models-transformed/vinyl-transformed.glb') as GLTFResult;
  const textures = useLoader(THREE.TextureLoader, texturesSrc);
  const instances = useRef<THREE.InstancedMesh>(null);
  const meshRefs = texturesSrc.map(() => useRef<THREE.Group>(null));

  const geometry = useMemo(() => new THREE.CircleGeometry(0.75, 25), []);
  const geometry2 = useMemo(() => new THREE.CylinderGeometry(0.75, 0.75, 0.1, 25), []);
  const invMaterial = useMemo(() => new THREE.MeshStandardMaterial({ visible: false }), []);

  const materials = useMemo(() => textures.map(texture => new THREE.MeshStandardMaterial({ map: texture, side: THREE.DoubleSide })), [textures]);

  useFrame(() => {
    if (!instances.current) return;

    instances.current.children
      .filter(instance => !!(instance as any).instance)
      .forEach((instance, i) => {
        if (!meshRefs[i]?.current) return;

        const p = new THREE.Vector3();
        const r = new THREE.Quaternion();

        meshRefs[i].current?.getWorldPosition(p);
        meshRefs[i].current?.getWorldQuaternion(r);

        instance.setRotationFromQuaternion(r);
        instance.rotateX(0.494);

        instance.position.set(p.x, p.y, p.z);
      });
  });

  return (
    <Instances ref={instances}>
      <bufferGeometry {...nodes.vinile.geometry} />
      <meshStandardMaterial roughness={1} metalness={0.4} color={'#000000'} side={THREE.DoubleSide} />

      {texturesSrc.map((_, i) => (
        <React.Fragment key={i}>
          <Instance scale={3.2} />
          <DraggableRigidBody
            {...props.draggableRigidBodyProps}
            groupProps={{ position: generatedPositions[i] }}
            rigidBodyProps={{ colliders: 'cuboid', density: 3 }}
            enableSpringJoint={true}
            visibleComponentRef={meshRefs[i]}
            visibleMesh={
              <group ref={meshRefs[i]}>
                <mesh scale={2} rotation={[Math.PI / 2, 0, 0]} geometry={geometry2} material={invMaterial} />
                <mesh scale={1.125} geometry={geometry} material={materials[i]} />
              </group>
            }
          />
        </React.Fragment>
      ))}
    </Instances>
  );
}));