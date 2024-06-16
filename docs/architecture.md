# System Architecture

## Overview
A comprehensive solution called "Safety Monitoring System and Black Box for Lending Vehicles" has been developed to enhance security measures and provide state-of-the-art real-time monitoring capability. This solution uses a carefully selected set of hardware components that work together to achieve its intended functions. The following sections describe the complex design, including the major hardware components, their interconnections, and their individual contributions to achieving the system's overarching goals.

## Microcontroller (Arduino)
Positioned at the heart of the system, the microcontroller acts as its cerebral cortex. Responsible for processing inputs from various sensors, controlling actuators, monitoring the touchscreen interface, and managing external device communication, it organizes multiple operations of the system.

## Sensors
- **Alcohol Sensor**: This sensor discerns the presence of alcohol in its vicinity, a crucial capability for detecting unauthorized alcohol consumption in restricted areas.
- **Flame Sensor**: Tasked with detecting fire or elevated temperatures, the flame sensor empowers the system to swiftly identify and mitigate potential fire hazards.
- **Temperature Sensor**: By gauging ambient temperature, this sensor contributes to environmental monitoring, facilitating safety assessments and interventions.
- **Rain Sensor**: Distinguished by its ability to detect water droplets, the rain sensor plays a pivotal role in automated wiper control, enhancing visibility during inclement weather.
- **Touch Sensor**: This sensor is used to turn the system on/off.

## Actuators
- **Buzzer and LED**: These components indicate the system's on/off status.
- **OLED Display**: The OLED display serves as the system's primary portal for user interaction, presenting real-time sensor data, alerts, and system statuses.

## Communication Modules
- **GSM Module**: The GSM module is tasked with dispatching SMS alerts to designated phone numbers in response to security breaches or anomalous activities.
- **Fingerprint Sensor**: Acting as a biometric guard, this module introduces an additional layer of security by authenticating users through fingerprint recognition, ensuring authorized access to system functionality.

## Motor Module and Motors
- **Motor**: Acts as the engine of the vehicle.
- **Wiper Motor**: Functions as the wiper of the vehicle.



