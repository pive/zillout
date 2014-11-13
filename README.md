zillout
=======

A simple music player for node.js

## Overview
**Zillout** is a simple personal web music server and player built on top of **node.js** that is meant to be run on _e.g._ a media server. Once installed and configured with the audio library location, clients will be able to play music from any web browser.

It can be run locally (localhost), within a local network and even on the internet.

## Components
**Zillout** is composed of:

- a node.js server native module that scans and lists the available tracks in the audio library (ID3 tags),
- a web application that displays ID3 info and serves the audio files for playback.

## Development status
**Zillout** is under development on Mac OS X. It will later on support Linux, and may be Windows.

- Audio library native node module
  - A first working version is commited (builds on OS X).
- Web application
  - node server: first working version serving library (OS X only).
  - client application: first version loading library files with minimal features.

## 3rd party code/libraries
**Zillout** uses:

  - libid3tag (libmad) - statically linked into the node.js module
  - zlib - statically linked into the node.js module
  - v8, node and node-gyp
  - A public-domain hash function by Bob Jenkins
  - node modules: express, socket.io and extend
  - javascript client libraries: angular.js, bootstrap, ui-bootstrap and soundmanager2.
