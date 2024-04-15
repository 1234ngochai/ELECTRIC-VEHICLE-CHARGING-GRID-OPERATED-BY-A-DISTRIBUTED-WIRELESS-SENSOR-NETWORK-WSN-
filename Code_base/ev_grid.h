#ifndef EV_GRID_H
#define EV_GRID_H

#include <mpi.h>

void simulate_ev_node(MPI_Comm grid_comm);
void send_report_to_base_station(int my_rank, int num_messages, int coords[2], int heavily_utilized_neighbors[4]);

#endif