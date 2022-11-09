# Multi device controller for Halloween decorations

This project controls multiple devices for a little bit of fun. Designed to control a guillotine and air solenoid after a victum triggers sensor

## Notes

- The limit switch logic is implemented but not tested
- Bluetooth audio output partially working dependent on end user device

## Ideas to improve

- Add DMX support to trigger smoke machine/lights/sound
- Implement limit switch support instead of time-based stepper movements

## Devices

| Device          | PIN | Purpose            |
| --------------- | --- | ------------------ |
| Stepper driver  | 22  | Direction          |
| Stepper driver  | 23  | Step               |
| Distance sensor | 4   | Trigger            |
| Limit switch    | 10  | Up direction       |
| Limit switch    | 10  | Down direction     |
| Air             | 32  | Relay for solenoid |
