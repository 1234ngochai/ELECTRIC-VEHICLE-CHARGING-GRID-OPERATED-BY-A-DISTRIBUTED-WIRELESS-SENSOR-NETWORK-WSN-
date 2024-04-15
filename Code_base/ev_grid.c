#include <stdio.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include <unistd.h> // for sleep function
#include <stdlib.h>
#include "ev_buffer.h"
#include "ev_grid.h"
#include <string.h>
#include <sys/time.h>

#define K 5  // number of ports
#define CYCLE_DURATION 2  // in seconds
#define MAX_RUN_TIME 30    // in seconds
#define THRESHOLD 2        // trigger a request if only 2 port is available
#define HEAVILY_UTILIZED_THRESHOLD 0

void simulate_ev_node(MPI_Comm grid_comm) {
    int rank;
    int coords[2];  // to store x,y coordinates
    int ports[K];  // 1 if available, 0 if in-use
    int available_ports;
    EVBuffer evBuffer;
    MPI_Request requests[8]; // max 4 for sending and 4 for receiving
    

    MPI_Comm_rank(grid_comm, &rank);
    MPI_Cart_coords(grid_comm, rank, 2, coords);
    
    setupEVBuffer(&evBuffer);  // Initialize the EV buffer

    // Initialize all ports to available
    for (int i = 0; i < K; i++) {
        ports[i] = 1;
    }

    time_t start_time, current_time;
    time(&start_time); // Initialize start time


    while(1) {

        // Update ports in parallel using OpenMP threads
        int num_messages_exchanged = 0;  // number of exchanged message

        #pragma omp parallel for num_threads(K)
        for (int i = 0; i < K; i++) {
            // Simulate a port being available or in-use (randomly)
            ports[i] = rand() % 2;
        }

        // Count available ports after each cycle
        available_ports = 0;
        for (int i = 0; i < K; i++) {
            available_ports += ports[i];
        }

        // Get the current time
        struct tm * time_info;
        time(&current_time);
        time_info = localtime(&current_time);

        // Create a PortData instance.
        PortData port_data;
        port_data.year = 1900 + time_info->tm_year;
        port_data.month = time_info->tm_mon + 1;
        port_data.day = time_info->tm_mday;
        port_data.hour = time_info->tm_hour;
        port_data.minute = time_info->tm_min;
        port_data.second = time_info->tm_sec;
        port_data.available_ports = available_ports;

        // Add the PortData instance to the EV buffer
        addDataToBuffer(&evBuffer, port_data);
        // Check if the available ports are below the threshold
        if (available_ports <= THRESHOLD) {
            int num_requests = 0;
            char *directions[] = {"top", "left", "bottom", "right"};
            
            int received_data_from_neighbors[4] = {-1, -1, -1, -1};  // Initialize to invalid values to later identify if data was received

            int potential_heavily_utilized_neighbors[4] = {-1, -1, -1, -1};// storing neighbour rank

            int heavily_utilized_count = 0;  // count of nodes including current that are heavily utilized
            int num_valid_neighbors = 0;

            for (int direction = 0; direction < 4; direction++) {  // top, bottom, left, right
                int source, dest;
                MPI_Cart_shift(grid_comm, direction % 2, (direction < 2) ? -1 : 1, &source, &dest);

                if (dest != MPI_PROC_NULL) {  // valid neighbour
                    potential_heavily_utilized_neighbors[direction] = dest;

                    num_messages_exchanged += 2;  // 1 for sending, 1 for receiving
                    num_valid_neighbors++;
                    // print out the request details
                    struct tm * time_info;
                    time(&current_time);
                    time_info = localtime(&current_time);
                    // printf("Node (%d,%d) at %04d-%02d-%02d %02d:%02d:%02d sends request to %s neighbour (Node %d).\n", 
                    //        coords[0], coords[1],
                    //        1900 + time_info->tm_year,
                    //        time_info->tm_mon + 1,
                    //        time_info->tm_mday,
                    //        time_info->tm_hour,
                    //        time_info->tm_min,
                    //        time_info->tm_sec,
                    //        directions[direction],
                    //        dest);

                    // send request to dest
                    MPI_Isend(&available_ports, 1, MPI_INT, dest, 0, grid_comm, &requests[num_requests]);
                    num_requests++;

                    // receive data from neighbour
                    MPI_Irecv(&received_data_from_neighbors[direction], 1, MPI_INT, dest, 0, grid_comm, &requests[num_requests]);
                    num_requests++;
                }
            }

            // Wait for all communications to complete
            MPI_Waitall(num_requests, requests, MPI_STATUSES_IGNORE);
            // int has_nearby_available_node = 0;
            for (int i = 0; i < 4; i++) {
                if (received_data_from_neighbors[i] != -1){
                    //check for nearest avaliable neighbour
                    if (received_data_from_neighbors[i] > HEAVILY_UTILIZED_THRESHOLD) {
                        // printf("Node (%d,%d): Nearest available neighbour is in %s direction.\n", coords[0], coords[1], directions[i]);
                    } else if (received_data_from_neighbors[i] <= HEAVILY_UTILIZED_THRESHOLD) {
                        heavily_utilized_count++;
                    }
                }
            }

            // add itself to the total
            if (available_ports <= HEAVILY_UTILIZED_THRESHOLD) {
                heavily_utilized_count++;
            }  
            // printf("Node (%d,%d) has %d valid neighbors.\n", coords[0], coords[1], num_valid_neighbors);
            // printf("Node (%d,%d) has %d heavily_utilized_count\n", coords[0], coords[1], heavily_utilized_count);

            if (heavily_utilized_count == num_valid_neighbors + 1) {  // All nodes including current are heavily utilized
                send_report_to_base_station(rank, num_messages_exchanged, coords,potential_heavily_utilized_neighbors);
            }
        }

        // Print node details, current date, time, and available ports
        // printf("Node (%d,%d): %04d %02d %02d %02d %02d %02d %d\n", 
        //        coords[0], coords[1],
        //        port_data.year,
        //        port_data.month,
        //        port_data.day,
        //        port_data.hour,
        //        port_data.minute,
        //        port_data.second,
        //        port_data.available_ports);
        int recvFlag;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &recvFlag, &status);
        if(recvFlag) {
            int terminationCmd;
            MPI_Recv(&terminationCmd, 1, MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD, &status);
            if(terminationCmd == -999) { 
            }
        }

        sleep(CYCLE_DURATION);
        // Update the current time after the sleep
        // Update the current time after the sleep
        time(&current_time);

        if (difftime(current_time, start_time) >= MAX_RUN_TIME) {
            break; // Exit the loop when the condition is met
            }
    }
    return;
}

void send_report_to_base_station(int my_rank, int num_messages, int coords[2], int heavily_utilized_neighbors[4]) {
    int base_station_rank = 0;

    // Getting current time details
    time_t current_time;
    time(&current_time);
    struct tm *time_info = localtime(&current_time);

    struct timeval current_timeval;
    gettimeofday(&current_timeval, NULL);
    int availableNode[9];

    char report_message[500];
    sprintf(report_message, "ALERT: Node %d at (%d,%d) - Heavily utilized quadrant detected on %04d-%02d-%02d %02d:%02d:%02d.%06ld. Messages exchanged: %d. Heavily utilized neighbors: ", 
            my_rank, 
            coords[0], coords[1], 
            1900 + time_info->tm_year, time_info->tm_mon + 1, time_info->tm_mday, 
            time_info->tm_hour, time_info->tm_min, time_info->tm_sec, 
            current_timeval.tv_usec,
            num_messages);
    
    // Append neighbor details to the report_message
    for (int i = 0; i < 4; i++) {
        if (heavily_utilized_neighbors[i] != -1) {
            char neighbor_info[50]; 
            sprintf(neighbor_info, "Node %d, ", heavily_utilized_neighbors[i]);
            strcat(report_message, neighbor_info);  // Append to the main message
        }
    }

    MPI_Send(report_message, strlen(report_message) + 1, MPI_CHAR, base_station_rank, 0, MPI_COMM_WORLD);
    MPI_Recv(availableNode, 9, MPI_INT, base_station_rank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // printf("received messga from the base station\n");
}
