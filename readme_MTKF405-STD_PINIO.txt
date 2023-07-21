# MatekF405-STD PINIO variant
- Replaces UART5 TX/RX pins by PINIO1 and PINIO2
(check /libraries/AP_HAL_ChibiOS/hwdef/MatekF405-STD/hwdef.dat for details)

- Configuration (parameter tree)
RCx_OPTION 28 (for Relay 1 - PINIO1 - PD2 - RX5 pin)
RCx_OPTION 34 (for Relay 2 - PINIO2 - PC12 - TX5 pin)

RELAY_PIN 81 (PINIO1)
RELAY_PIN2 82 (PINIO2)