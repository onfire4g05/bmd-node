export function getDeviceTemperature(deviceIndex: number): number;
export function getAvailableDevices(): number;
export function getDeviceOutputs(deviceIndex: number): number;

export type DuplexMode = 'full' | 'half' | 'simplex' | 'inactive' | 'unknown';
export type DuplexProfile = 'one-sub-full' | 'one-sub-half' | 'two-sub-full' | 'two-sub-half' | 'four-sub-half';

/** Get the current duplex mode of a device */
export function getDuplexMode(deviceIndex: number): DuplexMode;

/** Set the duplex profile for a device. Activation is asynchronous on the hardware side. */
export function setDuplexMode(deviceIndex: number, profile: DuplexProfile): boolean;

export type SdiLevel = 'A' | 'B';

/** Get SDI 3G output level. Returns 'A' or 'B'. */
export function getSdiLevel(deviceIndex: number): SdiLevel;

/** Set SDI 3G output level to 'A' or 'B'. Persists to device preferences. */
export function setSdiLevel(deviceIndex: number, level: SdiLevel): boolean;
