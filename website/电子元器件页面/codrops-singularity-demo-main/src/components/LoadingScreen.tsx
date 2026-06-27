import { CSSProperties, useEffect, useRef, useState } from 'react';
import { useProgress } from "@react-three/drei";

interface LoaderOptions {
    containerStyles: CSSProperties
    innerStyles: CSSProperties
    barStyles: CSSProperties
    dataStyles: CSSProperties
    dataInterpolation: (p: number) => string
    initialState: (active: boolean) => boolean
}

export default function LoadingScreen({
    containerStyles,
    innerStyles,
    initialState = (active: boolean) => active,
}: Partial<LoaderOptions>) {

    const { active, progress } = useProgress()
    const [shown, setShown] = useState(true)
    const [animation, setAnimation] = useState(false)

    const prevUpdate = useRef(0)
    const prevTimeout1 = useRef<ReturnType<typeof setTimeout> | undefined>(undefined);
    const prevTimeout2 = useRef<ReturnType<typeof setTimeout> | undefined>(undefined);

    // mostra il loader nel caso in cui il loader globale sia attivo
    useEffect(() => {
        let t: ReturnType<typeof setTimeout> | undefined
        if (active === true && shown === false) {
            t = setTimeout(() => {
                setAnimation(false)
                setShown(true)
            }, 300)
        }
        return () => t && clearTimeout(t)
    }, [shown, active])

    useEffect(() => {
        // sometimes progress is 100 and after that it goes back to 0 and then to 100 again
        if (prevTimeout2.current && (prevUpdate.current === 100 && progress === 0)) {
            clearTimeout(prevTimeout1.current)
            clearTimeout(prevTimeout2.current)
        }

        if (progress === 100) {
            prevTimeout1.current = setTimeout(() => setShown(false), 100 + 200)
            prevTimeout2.current = setTimeout(() => setAnimation(true), 200)
        }

        if (progress !== 0) {
            prevUpdate.current = progress;
        }

        return () => {
            prevTimeout1.current && clearTimeout(prevTimeout1.current)
            prevTimeout2.current && clearTimeout(prevTimeout2.current)
        }
    }, [progress])

    return shown ? (
        <div className='select-none transition-opacity duration-[100ms] ease-[cubic-bezier(1,.01,.9,.97)]' style={{ ...styles.container, opacity: animation ? 0 : 1, ...containerStyles }}>
            <div>
                <div style={{ ...styles.inner, ...innerStyles }} className='white'>
                    <p className='transition-all duration-[100ms] '
                        style={{
                            filter: 'drop-shadow(0px 0px 1px rgba(255, 255, 255, 0.727))',
                        }} />
                    Loading...
                    <p />
                </div>
            </div>
        </div >
    ) : null
}

const styles: { [key: string]: CSSProperties } = {
    container: {
        position: 'absolute',
        top: 0,
        left: 0,
        width: '100%',
        height: '100%',
        background: '#2e2e2e',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 1000,
    },
    inner: {
        textAlign: 'center',
    },
}