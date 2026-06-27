import type { Metadata } from "next";
import { GeistSans } from 'geist/font/sans';

import "./globals.css";
import Script from 'next/script';

export const metadata: Metadata = {
  title: "Singularity | Niccolò Fanton",
  description: "",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en" className={`${GeistSans.className} no-js h-auto antialiased bg-[#2e2e2e]`}>
      <head>
        <meta charSet="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <title>Singularity | Codrops</title>
        <meta name="description" content="" />
        <meta name="keywords" content="" />
        <meta name="author" content="Codrops" />
        <link rel="shortcut icon" href="favicon.ico" />

        <link rel="stylesheet" type="text/css" href="css/base.css" />
        <Script>
          document.documentElement.className="js";
        </Script>
        {/* <!--script src="//tympanus.net/codrops/adpacks/analytics.js"></script--> */}
      </head>
      <body className="demo-1 antialiased h-full">
        <main>
          <header className={`frame`}>
            {/* <h1 className="frame__title">Singularity</h1> */}
          
            {/* <nav className="frame__demos">
              <span>Variation 1</span>
              <a href="index2.html">Variation 2</a>
              <a href="index3.html">Variation 3</a>
            </nav> */}
          </header>
          <div className="content">
            {children}
          </div>
        </main>
        {/* <!--script src="https://tympanus.net/codrops/adpacks/cda_sponsor.js"></script--> */}
      </body>
    </html >
  );
}
