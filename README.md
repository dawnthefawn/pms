# Pebble Media Secretary

The Pebble Media Secretary (PMS) is designed to allow users to add media to a Sonarr/Radarr setup using a Pebble Smartwatch. PMS Takes a voice dictation, and sends the request to the specified server. 

PMS Now comes with a little node.js server and starter scripts (DONT USE THESE, THEY ARE NOT SECURE.) This allows you to send a request to your media server to restart your Sonarr or Radarr servers 
remotely using the watch app. It's just a little hack, sometimes sonarr can be a little unreliable for me, and the API wont work without a restart. 

## Getting Started

To install on your Pebble Smartwatch, simply clone the repository into your working directory, enable Pebble Developer Connection, and Install using the Pebble SDK.

To use the services-server-manager, copy the root directory somewhere onto your media server. Then modify the scripts found in services-server-manger/sh/ to configure them for your particular OS, 
and set the passwords. However, preferably you'd just abandon this script altogether as it stores your sudo password in plain text. You'll figure it out, you're a smart cookie.

### Prerequisites

You will need the Pebble SDK in order to install this Application. Please refer to the Pebble SDK documentation for information on how to do this.

In order to use server-services-manager, youll need node.js

## Authors

* **Dawn Nowell** - *Sole Author* - [dawnthefawn](https://github.com/dawnthefawn)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Special thanks to Pebble Clay for providing a configurator package.

