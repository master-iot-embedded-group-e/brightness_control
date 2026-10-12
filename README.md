# Brightness Control - Group E

Hands-on lab for Embedded Platforms and Communications for IoT.

## Members
- Omar Piñeiro
- Carlos Egea

## Gitflow conventions
- Follow standard gitflow for features
- `develop` is the default branch.
- Conventional Commits rules are followed.

## Features to develop
- `feat/fsm`: Finite State Machine that manages the business logic.
- `feat/short_long_button_press`: Feature to generate events from short and long presses
- `feat/pwm`: Feature to have brightness control 
- `feat/serial`: Feature for serial comms
- `feat/ldr`: Capture the LDR input and convert into luminosity (flexibility around the units)
- `feat/rgb`: Feature to control the RGB (might be developed inside another feature).

### How the tasks are assigned
Lottery:
- Omar
    - feat/serial
    - feat/ldr
    - feat/rgb
- Carlos
    - feat/fsm
    - feat/buttons
    - feat/pwm


## Hardware components
### LDR

Powered through a GPIO pin so it is only energized while a measurement is being taken. This minimizes power consumption and reduces unnecessary current draw when the sensor is idle. Additionally, the ADC is managed by the Device Runtime Power Management to power off the ADC when not in use in order to save energy.

| LDR pinout | Board pinout |
|---|---|
| `-` | GND |
| `+` | D2 |
| `S` | A0 |

### RGB

## Architecture
The architecture is based in threads:
- `main` thread for configuring and initializing the application. But it should not perform on this thread and should be idle most of the time.
- `producer` thread that gets the readings and events from the physical world.
- `actuator` thread that manages the rgb and serial interface
- `control` thread that handles the business logic of the application.

## Interfaces
To avoid problems between the threads, queued messages will be used. The reference for this can be found here: [Queued Messages](https://docs.zephyrproject.org/latest/kernel/services/data_passing/message_queues.html)

The interface is defined in `src/interface.h`, with one structure for producer data and another for actuator data.

## Testing

## References
- [Threads and Sync](https://www.zephyrproject.org/common-multithreading-problems-and-their-fixes/)
- [Thread manipulation](https://docs.zephyrproject.org/latest/samples/basic/threads/README.html)