# bmd-node

Node.js native bindings for selected Blackmagic DeckLink device status and configuration operations, plus a small command-line interface.

## Current Functionality

The native module currently exposes:

- `getAvailableDevices()` -> number of detected DeckLink devices
- `getDeviceOutputs(deviceIndex)` -> output-capability count (currently 0 or 1)
- `getDeviceTemperature(deviceIndex)` -> temperature in tenths of a degree C
- `getDuplexMode(deviceIndex)` -> one of: `full`, `half`, `simplex`, `inactive`, `unknown`
- `setDuplexMode(deviceIndex, profile)` -> set device profile
- `getSdiLevel(deviceIndex)` -> `A` or `B`
- `setSdiLevel(deviceIndex, level)` -> set `A` or `B`

TypeScript declarations are included in `index.d.ts`.

## CLI

A basic CLI is provided at `cli.js` and published as the `bmd-node` binary.

### Usage

```bash
bmd-node help
bmd-node devices [--json]
bmd-node get --device <index> --param <parameter> [--json]
bmd-node set --device <index> --param <parameter> --value <value> [--json]
```

### Parameters

Readable parameters:

- `temperature`
- `outputs`
- `duplex-mode`
- `sdi-level`

Writable parameters:

- `duplex-mode`
- `sdi-level`

Allowed set values:

- `duplex-mode`: `one-sub-full`, `one-sub-half`, `two-sub-full`, `two-sub-half`, `four-sub-half`
- `sdi-level`: `A`, `B`

### Examples

```bash
# List all devices and known values
bmd-node devices

# Read values for a specific device (0-based index)
bmd-node get -d 0 -p duplex-mode
bmd-node get -d 0 -p sdi-level
bmd-node get -d 0 -p temperature

# Set values for a specific device
bmd-node set -d 0 -p duplex-mode -v two-sub-half
bmd-node set -d 0 -p sdi-level -v A

# JSON output for scripts
bmd-node devices --json
bmd-node get -d 0 -p sdi-level --json
```

## Build Notes

This package uses `node-gyp` and links against the DeckLink SDK dispatch code and DeckLinkAPI library.

Install/build:

```bash
npm install
```

If you need to rebuild manually:

```bash
npm rebuild
```

## Limitations

- Requires DeckLink SDK headers/library and compatible runtime installation.
- Some functions depend on device support (for example, profile management or SMPTE Level A switching).
- Errors from native APIs are surfaced directly by the module/CLI.

## License

MIT. See `LICENSE`.
