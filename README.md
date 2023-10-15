
## Introduction

This project uses the [FrekvensPanel](https://github.com/frumperino/FrekvensPanel) project to control the **IKEA Frekvens Panel** with additional the _matrixled.ino_ to spin up a server to connect to your panel. 

## Getting Started

### Client - Next.js

First, run the development server:

```bash
npm run dev
# or
yarn dev
# or
pnpm dev
```

Open [http://localhost:3000](http://localhost:3000) with your browser to see the result.

### Server

Open FrekvensPanel.cpp, FrekvensPanel.h from [FrekvensPanel Project](https://github.com/frumperino/FrekvensPanel) and matrixled.ino (in hardware folder) in Arduino IDE. Now flash the file to a NodeMCU / esp8266 device with WLAN module.
