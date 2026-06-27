import * as THREE from 'three';
import { useThree, useFrame, ThreeElements, GroupProps } from '@react-three/fiber';
import { RapierRigidBody, RigidBody, RigidBodyProps, useSpringJoint } from '@react-three/rapier';
import React, { useState, useRef, useCallback, useMemo, forwardRef, ReactElement, useImperativeHandle } from 'react';
import { CustomDragControls, CustomDragControlsProps } from './CustomDragControls';

export const DEFAULT_SPRING_JOINT_CONFIG = {
    restLength: 0,
    stiffness: 100000,
    damping: 0,
    collisionGroups: 2
}

export interface DraggableRigidBodyProps {
    groupProps?: GroupProps,
    boundingBox?: [
        [number, number] | undefined,
        [number, number] | undefined,
        [number, number] | undefined
    ],
    dragControlsProps?: Partial<CustomDragControlsProps>,
    rigidBodyProps?: Partial<RigidBodyProps>,
    visibleComponentRef?: React.RefObject<THREE.Mesh | THREE.Group>,
    visibleMesh: ReactElement<ThreeElements['mesh']>,
    invisibleMesh?: ReactElement<ThreeElements['mesh']>,
    enableSpringJoint?: boolean,
    enabled?: boolean,
    jointConfig?: {
        restLength?: number,
        stiffness?: number,
        damping?: number,
        springJointCollisionGroups?: number,
    },
    instanceMode?: boolean,
    onDragStart?: () => void,
    onDragStop?: () => void,
}

interface DraggableRigidBodyRef {
    getInvisibleMesh: () => THREE.Mesh | null;
    getVisibleMesh: () => THREE.Mesh | null;
}

const DraggableRigidBody = React.memo(forwardRef<DraggableRigidBodyRef, DraggableRigidBodyProps>(
    (props, ref) => {
        const [isDragging, setIsDragging] = useState(false);
        const { scene } = useThree();

        const rigidBodyRef = useRef<RapierRigidBody>(null);
        const jointRigidBodyRef = useRef<RapierRigidBody>(null);
        const meshRef = useRef<THREE.Mesh>(null);
        const invisibleDragControlsMeshRef = useRef<THREE.Mesh>(null);

        useImperativeHandle(ref, () => ({
            getInvisibleMesh: () => invisibleDragControlsMeshRef.current,
            getVisibleMesh: () => meshRef.current,
        }));

        const springJointConfig = useMemo(() => ({
            restLength: props.jointConfig?.restLength ?? DEFAULT_SPRING_JOINT_CONFIG.restLength,
            stiffness: props.jointConfig?.stiffness ?? DEFAULT_SPRING_JOINT_CONFIG.stiffness,
            damping: props.jointConfig?.damping ?? DEFAULT_SPRING_JOINT_CONFIG.damping,
        }), [props.jointConfig]);

        useSpringJoint(
            jointRigidBodyRef,
            rigidBodyRef,
            [
                [0, 0, 0],
                [0, 0, 0],
                springJointConfig.restLength,
                springJointConfig.stiffness,
                springJointConfig.damping,
            ]
        );

        const startDragging = useCallback(() => {
            setIsDragging(true);
            if (props.onDragStart) props.onDragStart();

            if (props.enableSpringJoint && jointRigidBodyRef.current && rigidBodyRef.current) {
                jointRigidBodyRef.current.setBodyType(2, true);
                rigidBodyRef.current.setLinearDamping(10);
                rigidBodyRef.current.setAngularDamping(10);
                jointRigidBodyRef.current.wakeUp();
                return;
            }

            if (!rigidBodyRef.current) return;
            rigidBodyRef.current.setBodyType(2, true);
            rigidBodyRef.current.wakeUp();
        }, [props.onDragStart, props.enableSpringJoint]);

        const stopDragging = useCallback(() => {
            if (props.onDragStop) props.onDragStop();

            if (props.enableSpringJoint && jointRigidBodyRef.current && rigidBodyRef.current) {
                jointRigidBodyRef.current.setBodyType(0, true);
                rigidBodyRef.current.setLinearDamping(springJointConfig.damping);
                rigidBodyRef.current.setAngularDamping(springJointConfig.damping);
                setIsDragging(false);
                return;
            }

            if (!rigidBodyRef.current) return;
            rigidBodyRef.current.setBodyType(0, true);
            setIsDragging(false);
        }, [props.onDragStop, props.enableSpringJoint, springJointConfig.damping]);

        const onDrag = useCallback(() => {
            if (!isDragging || !rigidBodyRef.current || !invisibleDragControlsMeshRef.current) return;

            if (!props.enableSpringJoint && rigidBodyRef.current.bodyType() !== 2) return;
            if (props.enableSpringJoint && jointRigidBodyRef.current && jointRigidBodyRef.current.bodyType() !== 2) return;

            const position = new THREE.Vector3();
            invisibleDragControlsMeshRef.current.getWorldPosition(position);

            if (props.enableSpringJoint && jointRigidBodyRef.current) {
                jointRigidBodyRef.current.setNextKinematicTranslation(position);
                return;
            }

            rigidBodyRef.current.setNextKinematicTranslation(getBoxedPosition(position));
        }, [isDragging, props.enableSpringJoint]);

        useFrame(() => {
            if (jointRigidBodyRef.current && !jointRigidBodyRef.current.isSleeping() && !isDragging) {
                jointRigidBodyRef.current.setLinvel({ x: 0, y: 0, z: 0 }, false);
                jointRigidBodyRef.current.setAngvel({ x: 0, y: 0, z: 0 }, false);
            }

            if (!invisibleDragControlsMeshRef.current || (!meshRef.current && !props.visibleComponentRef?.current) || isDragging || rigidBodyRef.current?.bodyType() === 2 || rigidBodyRef.current?.isSleeping()) return;

            const ref = props.visibleComponentRef?.current ?? meshRef.current;
            const pmV = ref?.parent;
            const pmI = invisibleDragControlsMeshRef.current?.parent;

            if (!pmV || !pmI) return;

            scene.attach(ref);
            scene.attach(invisibleDragControlsMeshRef.current);

            const pos = ref.position;
            invisibleDragControlsMeshRef.current.position.set(pos.x, pos.y, pos.z);
            invisibleDragControlsMeshRef.current.setRotationFromEuler(ref.rotation);

            pmV.attach(ref);
            pmI.attach(invisibleDragControlsMeshRef.current);
        });

        const getBoxedPosition = useCallback((position: THREE.Vector3) => {
            if (!props.boundingBox) return position;

            const box = props.boundingBox;

            if (box[0]) {
                position.setX(Math.min(Math.max(box[0][0], position.x), box[0][1]));
            }

            if (box[1]) {
                position.setY(Math.min(Math.max(box[1][0], position.y), box[1][1]));
            }

            if (box[2]) {
                position.setZ(Math.min(Math.max(box[2][0], position.z), box[2][1]));
            }

            return position;
        }, [props.boundingBox]);

        return (
            <group {...props.groupProps}>
                {props.enableSpringJoint && (
                    <RigidBody type={'dynamic'} ref={jointRigidBodyRef} collisionGroups={props.jointConfig?.springJointCollisionGroups ?? DEFAULT_SPRING_JOINT_CONFIG.collisionGroups}
                        enabledRotations={[false, false, false]}
                        mass={1}
                        density={1}

                    >
                        <mesh>
                            <boxGeometry args={[.01, .01, .01]} />
                            <meshStandardMaterial visible={false} />
                        </mesh>
                    </RigidBody>
                )}

                {props.enabled !== false && (
                    <CustomDragControls onDragStart={startDragging} onDrag={onDrag} onDragEnd={stopDragging} {...props.dragControlsProps}>
                        {React.cloneElement(props.invisibleMesh ?? props.visibleMesh, { ref: invisibleDragControlsMeshRef, key: 'invisible', visible: false })}
                    </CustomDragControls>
                )}

                <RigidBody ref={rigidBodyRef} type={'dynamic'} colliders={'hull'} {...props.rigidBodyProps}>
                    {React.cloneElement(props.visibleMesh, { ref: (props.visibleComponentRef as any) ?? meshRef, key: 'visible' })}
                </RigidBody>
            </group>
        );
    }
));

export default DraggableRigidBody;