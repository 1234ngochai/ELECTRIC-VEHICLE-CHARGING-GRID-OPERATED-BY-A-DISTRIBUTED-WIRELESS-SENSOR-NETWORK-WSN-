# Electric Vehicle Charging Grid Simulation

## Project Overview
This project simulates a distributed Wireless Sensor Network (WSN) to manage an Electric Vehicle (EV) Charging Grid using Message Passing Interface (MPI) and POSIX/OpenMP in C or C++. It aims to optimize the allocation and navigation of EV charging stations based on real-time data to support the growth and convenience of electric vehicle usage.

## Background
With the increasing adoption of electric vehicles, efficient navigation to available EV charging stations is crucial. This simulation aims to optimize the selection and usage of these facilities, thereby aiding sustainable transportation efforts.

## System Description

### Architecture
The network architecture is divided into three primary sections: EV Charging Stations, Charging Ports, and the Base Station. Each charging station acts as a node within a distributed grid system, capable of communicating with neighboring nodes and a central Base Station which handles complex decision-making and system-wide alerts.

### Wireless Sensor Network (WSN)
The WSN consists of EV charging nodes arranged in a Cartesian grid, each represented by an MPI process. These nodes communicate with adjacent nodes and the central Base Station, managing real-time data on charging port availability.

### Node Configuration
Each node simulates multiple EV charging ports. Ports update their availability status to the charging station, which in turn can alert neighboring stations or the Base Station if it is nearing full capacity.

## Simulation Objectives
- **Distributed System Simulation:** Design and implement a WSN as a distributed computing system using MPI.
- **Parallel Algorithm Development:** Implement parallel algorithms to manage and process real-time data effectively.
- **Performance Analysis:** Evaluate the performance and scalability of the system under various loads.
- **Technical Communication:** Document and present the system design and outcomes comprehensively.

## Implementation Details

### Charging Port
Each charging station has several ports, each simulated by shared memory where threads represent port availability. This allows stations to quickly report their status to customers.

### EV Charging Station
Stations communicate within a grid system. If a station is full, it checks with neighbors. If neighbors are also full, it communicates with the Base Station.

### Base Station
Handles complex decisions, such as directing overflow traffic to the nearest available charging station. It only intervenes when necessary, maintaining system efficiency by avoiding unnecessary data transmission.

### Message Passing
Utilizes MPI SEND and RECV for communication. Blocking message passing ensures that messages are processed in order but can lead to delays if many alerts are triggered simultaneously.

### Single Computer vs Cluster Computing Setup
The simulation's performance differs significantly between single computer and cluster setups. Cluster computing allows more threads and processes to run simultaneously, greatly enhancing performance and scalability.

## Performance Considerations
The simulation shows that delays increase with the system's operation time due to the blocking nature of message passing. This aspect closely mirrors real-world scenarios where system load impacts responsiveness.

## Usage
Setup and operation instructions are provided within the code repository, tailored for different configurations and scenarios.

## Contributing
This project is open for academic and practical applications. Feedback and contributions are encouraged to enhance the simulation's accuracy and applicability.

## Additional Resources
Detailed explanations of the algorithms, system behaviors, and performance metrics are available in the accompanying project report, which includes methodologies, result tabulations, and analytical discussions.
