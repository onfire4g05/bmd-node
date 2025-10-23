const decklink = require('node-gyp-build')(__dirname);

module.exports = {
	getDeviceTemperature: decklink.getDeviceTemperature,
	getAvailableDevices: decklink.getAvailableDevices,
	getDeviceOutputs: decklink.getDeviceOutputs
};
