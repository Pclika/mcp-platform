import localFont from 'next/font/local';
import { GeistSans } from 'geist/font/sans';

export const KonstruktorFont = localFont({
  src: [
    {
      path: '../../public/fonts/KonstruktorRegular.otf',
      weight: '400',
      style: 'normal',
    }
  ],
  display: 'swap',
})

export const Branding = (props: any = { onShowSettings: () => { } }) => {
  return (
    <div className='fixed top-5 left-5 z-10 text-black/35'>

      <p style={{ filter: 'drop-shadow(0px 0px 10px rgba(118, 118, 118, 1))' }}
        className={`${KonstruktorFont.className} mb-[-10px] text-4xl sm:text-6xl sm:mb-[-15px] select-none`}>
        SINGULARITY
      </p>

      <p style={{ filter: 'drop-shadow(0px 0px 10px rgba(118, 118, 118, 1))' }} className={`${GeistSans.className} select-none text-sm`}>
        by <a href='https://x.com/niccolofanton' target='_blank' className='underline' >@niccolofanton</a>
      </p>

      <a
        className={`${GeistSans.className} fixed z-10 text-xs underline top-4 right-5 text-black/35`} onClick={() => props.onShowSettings()}>
        Show settings
      </a>

      <div className='flex gap-5 pt-3'>
        <a className="frame__back" href="https://tympanus.net/codrops/?p=86572">Read the tutorial</a>
        <a className="frame__github" href="https://github.com/niccolofanton/codrops-singularity-demo">GitHub</a>
        <a className="frame__archive" href="https://tympanus.net/codrops/demos/">All demos</a>
      </div>

      <nav className="frame__tags white fixed bottom-5 flex gap-5">
        <a href="https://tympanus.net/codrops/demos/?tag=react-three-fiber">#react-three-fiber</a>
        <a href="https://tympanus.net/codrops/demos/?tag=three-js">#three.js</a>
        <a href="https://tympanus.net/codrops/demos/?tag=webgl">#webgl</a>
      </nav>

    </div>
  );
};

