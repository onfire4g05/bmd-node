#!/usr/bin/env node

'use strict';

const VALID_GET_PARAMS = ['temperature', 'outputs', 'duplex-mode', 'sdi-level'];
const VALID_SET_PARAMS = ['duplex-mode', 'sdi-level'];

const VALID_DUPLEX_VALUES = [
  'one-sub-full',
  'one-sub-half',
  'two-sub-full',
  'two-sub-half',
  'four-sub-half'
];

const VALID_SDI_VALUES = ['A', 'B'];

let bmdNode;

function requireModule() {
  if (bmdNode) {
    return bmdNode;
  }

  try {
    bmdNode = require('./index.js');
    return bmdNode;
  } catch (error) {
    throw new Error(`Unable to load native module: ${error.message}`);
  }
}

function printHelp() {
  const helpText = `bmd-node CLI

Usage:
  bmd-node devices [--json]
  bmd-node get --device <index> --param <parameter> [--json]
  bmd-node set --device <index> --param <parameter> --value <value> [--json]
  bmd-node help

Commands:
  devices
    List devices and known parameter values.

  get
    Read one parameter from a specific device.
    Parameters: ${VALID_GET_PARAMS.join(', ')}

  set
    Write one parameter on a specific device.
    Settable parameters: ${VALID_SET_PARAMS.join(', ')}

Parameters:
  --device, -d   Device index (0-based)
  --param, -p    Parameter name
  --value, -v    New value for set command
  --json         Output JSON for scripting
  --help, -h     Show this help

Examples:
  bmd-node devices
  bmd-node get -d 0 -p duplex-mode
  bmd-node set -d 0 -p duplex-mode -v two-sub-half
  bmd-node set -d 1 -p sdi-level -v A
  bmd-node devices --json
`;

  process.stdout.write(helpText);
}

function printError(message) {
  process.stderr.write(`Error: ${message}\n`);
}

function parseArgs(argv) {
  const result = {
    _: [],
    flags: {}
  };

  for (let i = 0; i < argv.length; i++) {
    const token = argv[i];

    if (!token.startsWith('-')) {
      result._.push(token);
      continue;
    }

    if (token === '--json') {
      result.flags.json = true;
      continue;
    }

    if (token === '--help' || token === '-h') {
      result.flags.help = true;
      continue;
    }

    if (token === '--device' || token === '-d') {
      result.flags.device = argv[++i];
      continue;
    }

    if (token === '--param' || token === '-p') {
      result.flags.param = argv[++i];
      continue;
    }

    if (token === '--value' || token === '-v') {
      result.flags.value = argv[++i];
      continue;
    }

    throw new Error(`Unknown option: ${token}`);
  }

  return result;
}

function requireDeviceIndex(rawDevice) {
  const moduleRef = requireModule();

  if (rawDevice === undefined) {
    throw new Error('Missing --device <index>');
  }

  const deviceIndex = Number(rawDevice);
  if (!Number.isInteger(deviceIndex) || deviceIndex < 0) {
    throw new Error('Device index must be a non-negative integer');
  }

  const count = moduleRef.getAvailableDevices();
  if (deviceIndex >= count) {
    throw new Error(`Device index ${deviceIndex} is out of range (available: 0-${Math.max(0, count - 1)})`);
  }

  return deviceIndex;
}

function normalizeParam(rawParam) {
  if (!rawParam) {
    throw new Error('Missing --param <parameter>');
  }

  return String(rawParam).toLowerCase();
}

function getParameterValue(deviceIndex, param) {
  const moduleRef = requireModule();

  switch (param) {
    case 'temperature': {
      const raw = moduleRef.getDeviceTemperature(deviceIndex);
      return {
        parameter: 'temperature',
        value: raw,
        formatted: `${(raw / 10).toFixed(1)}C`,
        unit: '0.1C'
      };
    }
    case 'outputs':
      return { parameter: 'outputs', value: moduleRef.getDeviceOutputs(deviceIndex) };
    case 'duplex-mode':
      return { parameter: 'duplex-mode', value: moduleRef.getDuplexMode(deviceIndex) };
    case 'sdi-level':
      return { parameter: 'sdi-level', value: moduleRef.getSdiLevel(deviceIndex) };
    default:
      throw new Error(
        `Invalid get parameter: ${param}. Valid parameters: ${VALID_GET_PARAMS.join(', ')}`
      );
  }
}

function setParameterValue(deviceIndex, param, rawValue) {
  const moduleRef = requireModule();

  if (rawValue === undefined) {
    throw new Error('Missing --value <value>');
  }

  switch (param) {
    case 'duplex-mode': {
      const value = String(rawValue).toLowerCase();
      if (!VALID_DUPLEX_VALUES.includes(value)) {
        throw new Error(
          `Invalid duplex-mode value: ${rawValue}. Valid values: ${VALID_DUPLEX_VALUES.join(', ')}`
        );
      }
      moduleRef.setDuplexMode(deviceIndex, value);
      return { parameter: 'duplex-mode', value };
    }

    case 'sdi-level': {
      const value = String(rawValue).toUpperCase();
      if (!VALID_SDI_VALUES.includes(value)) {
        throw new Error(`Invalid sdi-level value: ${rawValue}. Valid values: ${VALID_SDI_VALUES.join(', ')}`);
      }
      moduleRef.setSdiLevel(deviceIndex, value);
      return { parameter: 'sdi-level', value };
    }

    default:
      throw new Error(
        `Invalid set parameter: ${param}. Settable parameters: ${VALID_SET_PARAMS.join(', ')}`
      );
  }
}

function listDevices(asJson) {
  const moduleRef = requireModule();
  const count = moduleRef.getAvailableDevices();
  const devices = [];

  for (let i = 0; i < count; i++) {
    const entry = { device: i };

    try {
      entry.outputs = moduleRef.getDeviceOutputs(i);
    } catch (error) {
      entry.outputsError = error.message;
    }

    try {
      entry.duplexMode = moduleRef.getDuplexMode(i);
    } catch (error) {
      entry.duplexModeError = error.message;
    }

    try {
      entry.sdiLevel = moduleRef.getSdiLevel(i);
    } catch (error) {
      entry.sdiLevelError = error.message;
    }

    try {
      const tempRaw = moduleRef.getDeviceTemperature(i);
      entry.temperatureRaw = tempRaw;
      entry.temperatureC = Number((tempRaw / 10).toFixed(1));
    } catch (error) {
      entry.temperatureError = error.message;
    }

    devices.push(entry);
  }

  if (asJson) {
    process.stdout.write(`${JSON.stringify({ deviceCount: count, devices }, null, 2)}\n`);
    return;
  }

  process.stdout.write(`Found ${count} DeckLink device(s).\n`);

  devices.forEach((device) => {
    process.stdout.write(`\nDevice ${device.device}\n`);
    process.stdout.write(`  outputs: ${formatField(device.outputs, device.outputsError)}\n`);
    process.stdout.write(`  duplex-mode: ${formatField(device.duplexMode, device.duplexModeError)}\n`);
    process.stdout.write(`  sdi-level: ${formatField(device.sdiLevel, device.sdiLevelError)}\n`);
    if (device.temperatureError) {
      process.stdout.write(`  temperature: error (${device.temperatureError})\n`);
    } else {
      process.stdout.write(`  temperature: ${device.temperatureC}C (${device.temperatureRaw} in tenths)\n`);
    }
  });
}

function formatField(value, error) {
  if (error) return `error (${error})`;
  if (value === undefined) return 'n/a';
  return String(value);
}

function main() {
  let parsed;

  try {
    parsed = parseArgs(process.argv.slice(2));
  } catch (error) {
    printError(error.message);
    printHelp();
    process.exit(1);
  }

  const command = parsed._[0];
  const asJson = Boolean(parsed.flags.json);

  if (!command || parsed.flags.help || command === 'help') {
    printHelp();
    return;
  }

  try {
    if (command === 'devices') {
      listDevices(asJson);
      return;
    }

    if (command === 'get') {
      const deviceIndex = requireDeviceIndex(parsed.flags.device);
      const param = normalizeParam(parsed.flags.param);
      const result = getParameterValue(deviceIndex, param);

      if (asJson) {
        process.stdout.write(`${JSON.stringify({ device: deviceIndex, ...result }, null, 2)}\n`);
      } else {
        const extra = result.formatted ? ` (${result.formatted})` : '';
        process.stdout.write(`device=${deviceIndex} ${result.parameter}=${result.value}${extra}\n`);
      }
      return;
    }

    if (command === 'set') {
      const deviceIndex = requireDeviceIndex(parsed.flags.device);
      const param = normalizeParam(parsed.flags.param);
      const result = setParameterValue(deviceIndex, param, parsed.flags.value);

      if (asJson) {
        process.stdout.write(`${JSON.stringify({ device: deviceIndex, status: 'ok', ...result }, null, 2)}\n`);
      } else {
        process.stdout.write(`device=${deviceIndex} set ${result.parameter}=${result.value} ok\n`);
      }
      return;
    }

    throw new Error(`Unknown command: ${command}`);
  } catch (error) {
    printError(error.message || String(error));
    process.exit(1);
  }
}

main();
