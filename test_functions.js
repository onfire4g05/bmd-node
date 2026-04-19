// Test file to demonstrate usage of the new functions
// Note: This won't run without proper DeckLink library installation

try {
    const bmdNode = require('./');

    console.log('Testing DeckLink device functions...');

    // Test 1: Get available devices
    try {
		const deviceCount = bmdNode.getAvailableDevices();
        console.log(`Found ${deviceCount} DeckLink devices`);

        // Test 2: If devices are available, check their outputs
        if (deviceCount > 0) {
            for (let i = 0; i < deviceCount; i++) {
                try {
                    const outputCount = bmdNode.getDeviceOutputs(i);
                    console.log(`Device ${i} has ${outputCount} output connection(s)`);

					console.log(`Device ${i} duplex mode: ${bmdNode.getDuplexMode(i)}`);
					console.log(`Device ${i} SDI level: ${bmdNode.getSdiLevel(i)}`);

					console.log(`Setting device ${i} duplex mode to 1 (if supported)`);
					bmdNode.setDuplexMode(i, 'two-sub-half');
					console.log(`Device ${i} duplex mode after setting: ${bmdNode.getDuplexMode(i)}`);

					console.log(`Setting device ${i} SDI level to 1 (if supported)`);
					bmdNode.setSdiLevel(i, 'A');
					console.log(`Device ${i} SDI level after setting: ${bmdNode.getSdiLevel(i)}`);
                } catch (error) {
                    console.log(`Error getting outputs for device ${i}: ${error.message}`);
                }
            }
        } else {
            console.log('No DeckLink devices found.');
        }

        // Test 3: Test temperature function for comparison
        if (deviceCount > 0) {
            try {
                const temperature = bmdNode.getDeviceTemperature(0);
                console.log(`Device 0 temperature: ${temperature / 10}°C`);
            } catch (error) {
                console.log(`Error getting temperature for device 0: ${error.message}`);
            }
        }

    } catch (error) {
        console.log(`Error: ${error.message}`);
    }

} catch (error) {
    console.log('Module not available (likely due to linking issues):', error.message);
    console.log('This is expected if DeckLink libraries are not properly installed.');
}