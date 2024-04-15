
#include <stdio.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include <unistd.h> // for sleep function
#include <stdlib.h>
#include <string.h>
#include "ev_buffer.h"
#include "ev_grid.h"
#include <sys/time.h> // Include necessary header for gettimeofday

#define K 5  // number of ports
#define TIMEOUT 22 // seconds
#define GRID_SIZE 3

void findNearbyNodes(int rank);


// List of nodes
int nearby[GRID_SIZE * GRID_SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
int filledList[GRID_SIZE * GRID_SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
int reportedNode[GRID_SIZE * GRID_SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
int availableNode[GRID_SIZE * GRID_SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

int main(int argc, char *argv[]) {
    int world_rank, world_size;
    int grid_rank, grid_size;
    int colour;
    int is_main;
    time_t current_time, start_time;

    int dims[2] = {3, 3};     // 3x3 grid
    int periods[2] = {0, 0};  // non-periodic in both dimensions
    MPI_Comm grid_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank == 0)
    {
        colour = 0;
        is_main = 1;  // true
    }
    else
    {
        colour = 1;
        is_main = 0;  // false

    }

    MPI_Comm main_comm;
    MPI_Comm_split(MPI_COMM_WORLD, colour, world_rank, &main_comm);
    MPI_Comm_size(main_comm, &grid_size);
    MPI_Comm_rank(main_comm, &grid_rank);


    if (is_main == 0)  // If not the main/base station
    {
        MPI_Cart_create(main_comm, 2, dims, periods, 1, &grid_comm);
        simulate_ev_node(grid_comm);
    }
    else {  // if it's the base station
        char received_report[1000];  // buffer to receive the report
        MPI_Status status;

        int incoming_message_size;

        time_t last_reset_time; // Time at which the filledList was last reset
        time(&last_reset_time);

        time(&start_time);
        do {
            struct tm * time_info;
            time(&current_time);
            time_info = localtime(&current_time);
            
            // Use MPI_Test to check if a message is available without blocking.
            
            int flag;
            MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                // If a message is available, receive it.
                MPI_Recv(received_report, 1000, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
                printf("BASE STATION received report: %s\n", received_report);

                long usec; // to capture microseconds
                int node_rank, coord_x, coord_y, num_msg_exchanged, year, month, day, hour, minute, second;
                sscanf(received_report, "ALERT: Node %d at (%d,%d) - Heavily utilized quadrant detected on %d-%d-%d %d:%d:%d.%06ld. Messages exchanged: %d", 
                    &node_rank, &coord_x, &coord_y, &year, &month, &day, &hour, &minute, &second, &usec, &num_msg_exchanged);
                
                struct timeval current_time_microsec;
                gettimeofday(&current_time_microsec, NULL);
                long communication_time_usec = current_time_microsec.tv_usec - usec;

                printf("Communication time (in microseconds) between node and base-station: %ld\n", communication_time_usec);
                
                printf("========================================\n");
                printf("Reported Node\tNearby Nodes\n");
                printf("----------------------------------------\n");
                printf("Node %d at (%d,%d)\n", node_rank, coord_x, coord_y);
                printf("----------------------------------------\n");

                reportedNode[node_rank] = 0;
                filledList[node_rank] = 0; // Mark the node as filled
                int extracted_neighbors[10]; 
                int neighbors_count = 0;     // Counter for how many neighbors were extracted

                // Find the starting point of neighbors list in the report
                char *neighbors_start = strstr(received_report, "Heavily utilized neighbors:");

                if (neighbors_start) {
                    // Extracting the neighbor nodes
                    char *token = strtok(neighbors_start, " ,");  // Initialize strtok with the neighbors_start and split by space and comma
                    while (token != NULL) {
                        // If token starts with "Node", the next token is the node number
                        if (strcmp(token, "Node") == 0) {
                            token = strtok(NULL, " ,");  // Get the node number
                            int neighbor_node;
                            sscanf(token, "%d", &neighbor_node);
                            
                            filledList[neighbor_node] = 0; // Mark the neighbor node as filled
                            reportedNode[neighbor_node] = 0;
                            findNearbyNodes(neighbor_node);
                            
                            // Store the neighbor node in the array
                            extracted_neighbors[neighbors_count++] = neighbor_node;
                        }
                        token = strtok(NULL, " ,");  // Move to the next token
                    }
                }

                for (int i = 0; i < neighbors_count; i++) {
                    printf("Node %d\n", extracted_neighbors[i]);
                }

                printf("----------------------------------------\n");
                printf("Nearby nodes for Node %d: ", node_rank);
                int avaliable = 0;
                for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
                    if (nearby[i] == 0 && reportedNode[i] == -1) {
                        printf("Node %d, ", i);
                    }
                }
                
                printf("\n----------------------------------------\n");
                printf("Nearby avaliable nodes for Node(for the last 1 second) %d: ", node_rank);
                for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
                    if (nearby[i] == 0 && reportedNode[i] == -1 && filledList[i] == -1) {
                        printf("Node %d, ", i);
                        avaliable = 1;
                        availableNode[i] = 0;
                    }
                    nearby[i] = -1; // reset for the next iteration
                    reportedNode[i] = -1;
                }

                MPI_Send(availableNode, GRID_SIZE * GRID_SIZE, MPI_INT, node_rank+1, 1, MPI_COMM_WORLD);
                
                if(!avaliable){
                    printf("No near avaliable node");
                }
                printf("\n========================================\n");
            }


            time(&current_time);
            if (difftime(current_time, last_reset_time) >= 1.0) {
                // for (int i = 0; i < 9; i++) {
                //     if (filledList[i] != -1) {
                //         printf("Node %d: %d\n", i, filledList[i]);
                //     }
                // }
                // printf("\n"); // For separation
                for (int i = 0; i < 9; i++) {
                    filledList[i] = -1;
                }
                // Reset the last_reset_time for the next cycle
                time(&last_reset_time);
            }
            usleep(10000); // sleep for 10 milliseconds
        } while (difftime(current_time, start_time) < TIMEOUT);
    }


    int terminationFlag = -999; // flag to indicate termination
    for(int i=1; i<GRID_SIZE * GRID_SIZE; i++) { 
        // printf("Sent terminated message to all the node");
        MPI_Send(&terminationFlag, 1, MPI_INT, i, 2, MPI_COMM_WORLD); // Tag 2 is for termination commands.
        }
    MPI_Finalize();
    return 0;
}

void findNearbyNodes(int rank) {
    int x = rank / GRID_SIZE;
    int y = rank % GRID_SIZE;

    // Check top neighbor
    if (x > 0) {
        nearby[(x-1)*GRID_SIZE + y] = 0;
    }
    // Check bottom neighbor
    if (x < GRID_SIZE-1) {
        nearby[(x+1)*GRID_SIZE + y] = 0;
    }
    // Check left neighbor
    if (y > 0) {
        nearby[x*GRID_SIZE + (y-1)] = 0;
    }
    // Check right neighbor
    if (y < GRID_SIZE-1) {
        nearby[x*GRID_SIZE + (y+1)] = 0;
    }
}