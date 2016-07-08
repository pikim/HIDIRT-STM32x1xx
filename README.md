# HIDIRT-STM32x1xx
Device firmware for HIDIRT (HID InfraRed Transceiver) written in C for STM32 (F1 or L1).
It uses ST's HAL libraries and was developed on Eclipse CDT (Luna) with the GNU ARM Eclipse plugin. Eclipse configurations are used to select whether the target is a F1 or a L1 device.

The project uses a software based RTC, because not all devices support fine calibration on the RTC peripheral.
