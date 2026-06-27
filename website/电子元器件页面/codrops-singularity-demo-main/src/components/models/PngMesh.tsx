import { forwardRef } from 'react';
import { useLoader, GroupProps } from '@react-three/fiber';
import * as THREE from 'three';

interface TexturedPlaneProps extends GroupProps {
    src: string;
    scale?: number;
}

const TexturedPlane = forwardRef<THREE.Group, TexturedPlaneProps>(
    ({ src, scale = 1, ...props }, ref) => {
        const texture = useLoader(THREE.TextureLoader, src);

        return (
            <group {...props} ref={ref}>
                <mesh>
                    <planeGeometry args={[3, 3]} />
                    <meshBasicMaterial map={texture} transparent side={THREE.DoubleSide} />
                </mesh>

                <mesh>
                    <boxGeometry args={[3, 3, .2]} />
                    <meshBasicMaterial visible={false} />
                </mesh>
            </group>
        );
    }
);

export default TexturedPlane;
