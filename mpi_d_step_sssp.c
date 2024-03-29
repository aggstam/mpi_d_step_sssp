// -------------------------------------------------------
//
// This program implements D-Stepping algorithm
// introduced by U.Mayers and P.Sanders.
// Graph is read from an input file created by
// RandomGraph generator by S.Pettie and V.Ramachandran.
// Finding shortest path distance for each node occurs in
// parallel, by using the MPI Standard.
//
// Author: Angelos Stamatiou, January 2020
//
// -------------------------------------------------------

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Bucket structure
struct bucket {
    int *delta;
    int *nodes;
};

double d_step;          // D value used.
FILE *fin;              // Input file.
FILE *fout;             // Output file.
int nodes_count;        // Graph nodes count.
double **matrix;        // Graph nodes matrix.
struct bucket *buckets; // Buckets array.
double **distances;     // Shortest path distance from each node to all other nodes.
int *light_nodes;       // Light nodes array.
int *heavy_nodes;       // Heavy nodes array.
int *visited_nodes;     // Visited nodes array, used to prevent loops.
int *updated_nodes;     // Updated nodes array, used to also update visited neighbors.
int source_node;        // Source node to find distances.
clock_t t1, t2;         // Variables for measuring time spent.

// This function inserts a given value at the end of a given array.
// Inputs:
//      int *array: The array.
//      int node: The value to insert.
void push_node(int *array, int node)
{
    int pos = 0;    
    int item = array[0];    
    while (item != node && item != -1) {
        pos ++;
        item = array[pos];        
    }
    array[pos] = node;
}

// This function removes all values of a given array.
// Inputs:
//      int *array: The array.
void empty_array(int *array)
{
    int pos = 0;    
    int item = array[pos];    
    while (item != -1) {
        array[pos] = -1;
        pos ++;
        item = array[pos];        
    }    
}

// This function checks if a given array contains a given value.
// Inputs:
//      int *array: The array.
//      int node: The value to check.
// Output:
//      1 --> The array contains the value.
//      0 --> The array doesn't contains the value.
int contains(int *array, int node)
{
    int pos = 0;    
    int item = array[pos];    
    while (item != node && item != -1) {
        pos ++;
        item = array[pos];        
    }
    return array[pos] == node;
}

// This functions cleanses all empty buckets so they can be used again. 
// Also it will return the first non empty bucket or the last one,
// in case all are empty.
// Output:
//      struct bucket --> Returned bucket.
struct bucket find_next_bucket()
{
    for (int i = 0; i < nodes_count; i++) {
        if (buckets[i].nodes[0] == -1) {
            buckets[i].delta[0] = -1;        
        }    
    }
    int pos = 0;
    struct bucket item = buckets[pos];
    while (item.delta[0] == -1 && pos + 1 < nodes_count) {
        pos ++;
        item = buckets[pos];    
    }
    return item;
}

// This functions retrieves the bucket for a given D value.
// If the bucket doesn't exists, it assigns it to a cleansed one.
// Inputs:
//      int d: The D value to search.
// Output:
//      struct bucket --> Returned bucket.
struct bucket retrieve_bucket(int d)
{
    struct bucket b;
    b.delta = (int*)malloc(sizeof(int));
    if (b.delta == NULL) {
        printf("Error: b.delta for mult failed.\n");
        exit(1);
    }
    b.delta[0] = -1;
    for (int i = 0; i < nodes_count; i++) {
        if (buckets[i].delta[0] == d) {
            b = buckets[i];    
            break;    
        }    
    }
    if (b.delta[0] != d) {
        int pos = 0;
        b = buckets[0];
        while (b.delta[0] != -1) {
            pos ++;
            b = buckets[pos];    
        }
        b.delta[0] = d;        
    }
    return b;
}

// This functions add nodes contained in a given bucket to the 
// Visited nodes array and relaxes(insert to appropriate bucket) nodes 
// contained in a given array.
// Inputs:
//      int *bucket_nodes: The bucket.
//      int *relax_nodes: The array to relax.
void relax_nodes(int *bucket_nodes, int *relax_nodes)
{
    int pos = 0;    
    int item = bucket_nodes[0];
    
    // Add buckets nodes to Visited nodes array in order to prevent loops.
    while (item != -1) {        
        push_node(visited_nodes, item);
        pos ++;
        item = bucket_nodes[pos];    
    }
    empty_array(bucket_nodes);
    
    struct bucket node_bucket;
    int node_delta;
    pos = 0;    
    item = relax_nodes[0];    
    while (item != -1) {
        node_delta = distances[source_node][item] / d_step; // Find node Delta value.
        node_bucket = retrieve_bucket(node_delta); // Retrieve bucket for that Delta value.    
        if (!contains(visited_nodes, item)) { // Prevent loops.
            push_node(node_bucket.nodes, item);
        }    
        pos ++;
        item = relax_nodes[pos];    
    }
    pos = 0;    
    item = updated_nodes[0];    
    while (item != -1) { 
        node_delta = distances[source_node][item] / d_step; 
        node_bucket = retrieve_bucket(node_delta);        
        push_node(node_bucket.nodes, item); // Insert updated nodes in order to revisit neighbors.
        pos ++;
        item = updated_nodes[pos];
    }
    empty_array(updated_nodes);    
}

// This function resets arrays used by the program.
void reset_node_structures()
{
    for (int i = 0; i < nodes_count; i++) {
        heavy_nodes[i] = -1;
        light_nodes[i] = -1;
        visited_nodes[i] = -1;
        updated_nodes[i] = -1;
    }
}

// This function allocates memory needed for the Graph matrix.
void allocate_matrix()
{
    matrix = (double**)malloc(sizeof(double*) * nodes_count + sizeof(double) * nodes_count * nodes_count);
    if (matrix == NULL) {
        printf("Error: malloc for matrix failed.\n");
        exit(1);
    }
    double *ptr = (double*)(matrix + nodes_count);
    for(int i = 0; i < nodes_count; i++) {
        matrix[i] = (ptr + nodes_count * i);
    }            
}

// This function initializes the Graph matrix, by reading the input file.
void initialize_matrix()
{
    int i, j;
    double w;

    matrix = (double**)malloc(sizeof(double*) * nodes_count + sizeof(double) * nodes_count * nodes_count);
    if (matrix == NULL) {
        printf("Error: malloc for matrix failed.\n");
        exit(1);
    }
    double *ptr = (double*)(matrix + nodes_count);
    for(i = 0; i < nodes_count; i++) {
        matrix[i] = (ptr + nodes_count * i);
        for (j = 0; j < nodes_count; j++) {
            matrix[i][j] = -1;            
        }
    }            
    fscanf(fin, "%d", &i);
    while (i != -1) {
        fscanf(fin, "%d %lf \n", &j, &w);
        if (i != j && (matrix[i][j] == -1 || w < matrix[i][j])) {
            matrix[i][j] = w;
            matrix[j][i] = w;
        }                
        fscanf(fin, "%d", &i);
    }            
}

// This function initializes all arrays used by the program.
void initialize_structures()
{
    buckets = (struct bucket *)malloc(nodes_count * sizeof( struct bucket));
    if (buckets == NULL) {
        printf("Error: malloc for buckets failed.\n");
        exit(1);
    }
    distances = (double**)malloc(sizeof(double*) * nodes_count + sizeof(double) * nodes_count * nodes_count);
    if (distances == NULL) {
        printf("Error: malloc for distances failed.\n");
        exit(1);
    }
    double *ptr = (double*)(distances + nodes_count);
    heavy_nodes = (int*)malloc(nodes_count * sizeof(int));
    if (heavy_nodes == NULL) {
        printf("Error: malloc for heavy_nodes failed.\n");
        exit(1);
    }
    light_nodes = (int*)malloc(nodes_count * sizeof(int));
    if (light_nodes == NULL) {
        printf("Error: malloc for light_nodes failed.\n");
        exit(1);
    }
    visited_nodes = (int*)malloc(nodes_count * sizeof(int));
    if (visited_nodes == NULL) {
        printf("Error: malloc for visited_nodes failed.\n");
        exit(1);
    }
    updated_nodes = (int*)malloc(nodes_count * sizeof(int));
    if (updated_nodes == NULL) {
        printf("Error: malloc for updated_nodes failed.\n");
        exit(1);
    }
    for (int i = 0; i < nodes_count; i++) {
        buckets[i].delta = (int*)malloc(sizeof(int));
        if (buckets[i].delta == NULL) {
            printf("Error: malloc for buckets[%d].delta failed.\n", i);
            exit(1);
        }
        buckets[i].delta[0] = -1;
        buckets[i].nodes = (int*)malloc(nodes_count * sizeof(int));
        if (buckets[i].nodes == NULL) {
            printf("Error: malloc for buckets[%d].nodes failed.\n", i);
            exit(1);
        }
        distances[i] = (ptr + nodes_count * i);
        for (int j = 0; j < nodes_count; j++) {
            buckets[i].nodes[j] = -1;
            distances[i][j] = -1;            
        }        
        heavy_nodes[i] = -1;
        light_nodes[i] = -1;
        visited_nodes[i] = -1;
        updated_nodes[i] = -1;        
    }
}

// This function frees allocated memory of all arrays used by the program.
void free_structures()
{
    free(matrix);
    free(buckets);
    free(distances);
    free(heavy_nodes);
    free(light_nodes);
    free(visited_nodes);
    free(updated_nodes);
}

// This function implements the D-Stepping algorithm and finds 
// the shortest path distances from a source node to all other nodes.
void d_step_algorithm()
{
    int i, j;

    // First bucket to use will contain the source node and will
    // have Delta value 0.
    struct bucket current_bucket = buckets[0];
    current_bucket.delta[0] = 0;
    push_node(current_bucket.nodes, source_node);

    // While Buckets are not empty...
    while (current_bucket.nodes[0] != -1) {                
        i = 0;
        while (current_bucket.nodes[i] != -1) {    // For each bucket node...        
            for (j = 0; j < nodes_count; j++) { // For each neighbor...
                if (matrix[current_bucket.nodes[i]][j] != -1 && j != source_node) {
                    // Retrieve neighbor distance.
                    double temp_distance = matrix[current_bucket.nodes[i]][j];
                    // Insert in appropriate heavy-light node array.
                    if (temp_distance > d_step) {
                        push_node(heavy_nodes, j);
                    } else {
                        push_node(light_nodes, j);
                    }
                    // Add path-so-far distance for that node.
                    if (distances[source_node][current_bucket.nodes[i]] != -1) {
                        temp_distance += distances[source_node][current_bucket.nodes[i]];
                    }
                    // If the distance found is shorter that the saved one, update Distances array.                
                    if (distances[source_node][j] == -1 || temp_distance < distances[source_node][j]) {
                        distances[source_node][j] = temp_distance;
                        push_node(updated_nodes, j); // Insert node to Updated nodes array.
                    }            
                }                    
            }
            i ++;    
        }
        if (light_nodes[0] != -1) {
            relax_nodes(current_bucket.nodes, light_nodes); // Relax Light nodes first.
            empty_array(light_nodes);            
        } else {
            relax_nodes(current_bucket.nodes, heavy_nodes); // Relax Heavy nodes no light nodes exists.
            empty_array(heavy_nodes);            
        }

        current_bucket = find_next_bucket(); // Retrieve next bucket to process.
    }
}

// Auxiliary function that displays a message in case of wrong input parameters.
// Inputs:
//      char *compiled_name: Programms compiled name.
void syntax_message(char *compiled_name)
{
    printf("Correct syntax:\n");
    printf("%s <d_step> <input-file> <output-file>\n", compiled_name);
    printf("where: \n");
    printf("<d_step> is the desired D positive double that will be used by the algorithm.\n");
    printf("<input-file> is the file containing a generated graph by RandomGraph that the algorithm will use.\n");
    printf("<output-file> is the file shortest path distances for each node will be written.\n");
}

// This function checks run-time parameters validity and
// retrieves D-step value, input and output file names.
// Inputs:
//      char **argv: The run-time parameters.
// Output:
//      1 --> Parameters read succussfully.
//      0 --> Something went wrong.
int read_parameters(char **argv)
{
    char *d_step_string = argv[1];
    if (d_step_string == NULL) {
        printf("D-step parameter missing.\n");
        syntax_message(argv[0]);
        return 0;        
    }
    d_step = atof(d_step_string);
    if (d_step <= 0.0) {
        printf("Unable to process D-step.\n");
        syntax_message(argv[0]);
        return 0;
    }
    char *input_filename = argv[2];
    if (input_filename == NULL) {
        printf("Input file parameter missing.\n");
        syntax_message(argv[0]);
        return 0;
    }
            
    fin = fopen(input_filename, "r");
    if (fin == NULL) {
        printf("Cannot open input file %s.\n", input_filename);
        return 0;        
    }
    char *output_filename = argv[3];
    if (output_filename == NULL) {
        printf("Output file parameter missing.\n");
        syntax_message(argv[0]);
        return 0;
    }
            
    fout = fopen(output_filename, "w");
    if (fout == NULL) {
        printf("Cannot open outputfile %s.\n", output_filename);
        return 0;        
    }
    printf("Finding shortest path distances for each node of a Graph using D-Stepping algorithm.\n");
    printf("D-Stepping value: %lf\n", d_step);
    printf("Graph will be retrieved from input file: %s\n", input_filename);
    printf("Distances matrix will be written in output file: %s\n", output_filename);
    return 1;
}

// This function writes the found distances matrix to the output file.
// First line contains the nodes count.
// Last line contains -1 as EOF char.
// Inputs:
//      double **mpi_distances: Calculated distances.
//      double **mpi_remainder_distances: Calculated distances of remaining nodes(uneven distribution).
//      int remainder: Count of remaining nodes(uneven distribution).
void write_distances_to_file(double **mpi_distances, double **mpi_remainder_distances, int remainder)
{
    int i,j;
    int finish = nodes_count-remainder;
    fprintf(fout, "%d\n", nodes_count);
    for (i = 0; i < finish; i++) {
        for (j = 0; j < nodes_count; j++) {
            fprintf(fout, "%lf ", mpi_distances[i][j]);        
        }
        fprintf(fout, "\n");
    }
    if (remainder > 0) {
        // Remainders distance matrix is in reverse (P0 got line n, P1 got line n-1, etc.).
        for (i = remainder-1; i > -1; i--) {
            for (j = 0; j < nodes_count; j++) {
                fprintf(fout, "%lf ", mpi_remainder_distances[i][j]);        
            }
            fprintf(fout, "\n");
        }
    }
    fprintf(fout, "-1");
}

// This function handles remaining nodes(unven distribution).
// P0 collects remaining distances finalizes program by
// writing all distances to the output file.
// Inputs:
//      double **mpi_distances: Calculated distances.
//      int rank: Process rank.
//      int remainder: Count of remaining nodes(uneven distribution).
void handle_remainders_and_finalize(double **mpi_distances, int rank, int remainder)
{
    int i;
    double** mpi_remainder_distances;
    if (remainder > 0) {
        // Process 0 will collect all calculated distances by the assigned processes.        
        if (rank == 0) {
            mpi_remainder_distances = (double**)malloc(sizeof(double*) * remainder + sizeof(double) * remainder * nodes_count);
            if (mpi_remainder_distances == NULL) {
                printf("Error: malloc for mpi_remainder_distances failed.\n");
                exit(1);
            }
            double *mpi_remainder_distances_ptr = (double*)(mpi_remainder_distances + remainder);
            for (i = 0; i < remainder; i++) {
                mpi_remainder_distances[i] = (mpi_remainder_distances_ptr + nodes_count * i);        
            }
            for (i = 0; i < nodes_count; i++) {
                mpi_remainder_distances[0][i] = distances[nodes_count-1][i];    
            }
            for (i = 1; i < remainder; i++) {
                MPI_Recv(mpi_remainder_distances[i], nodes_count, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        } else {
            double *mpi_process_remainder_distances = (double*)malloc(nodes_count * sizeof(double));
            if (mpi_process_remainder_distances == NULL) {
                printf("Error: malloc for mpi_process_remainder_distances failed.\n");
                exit(1);
            }
            for (i = 0; i < nodes_count; i++) {
                mpi_process_remainder_distances[i] = distances[nodes_count-rank-1][i];        
            }
            MPI_Send(mpi_process_remainder_distances, nodes_count, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
            free(mpi_process_remainder_distances);
        }
    }

    if (rank == 0) {
        printf("Algorithm finished!\n");
        printf("Time spend: %f secs\n", ((float)t2 -t1) / CLOCKS_PER_SEC);
        printf("Writing distances to output file.\n");
        write_distances_to_file(mpi_distances, mpi_remainder_distances, remainder);
        free(mpi_distances);
        if (remainder > 0) {
            free(mpi_remainder_distances);
        }
    }
    free_structures();
}

// This function parallilizes the D-Stepping algorithm using the MPI Standard. 
// Each process calculate the shortest path distances for nodes count / processes count.
// If the distribution is un-even, remaining nodes will be assigned to a process in reverse.
// Inputs:
//      int rank: Process rank.
//      int size: Processes size.
void mpi_d_step(int rank, int size)
{
    int i,j;
    int interval =     nodes_count / size; // Each node will process interval nodes.
    int remainder = nodes_count % size; // Remaining nodes will be distributed evenly among process.
    int start = rank * interval;
    int finish = start + interval;
    initialize_structures();
    
    if (rank == 0) {
        t1 = clock();    
    }
    // Retrieve shortest path distances from each process node to all other nodes of the Graph.
    for (source_node = start; source_node < finish; source_node++) {
        d_step_algorithm();
        reset_node_structures();
    }
    // Retrieve shortest path for remaining node.
    if (rank < remainder) {
        source_node = nodes_count-rank-1; // Assign remaining node.
        d_step_algorithm();
        reset_node_structures();
    }    
    if (rank == 0) {
        t2 = clock();    
    }

    // Process 0 will collect all calculated distances.
    double **mpi_process_distances = (double**)malloc(sizeof(double*) * interval + sizeof(double) * interval * nodes_count);
    if (mpi_process_distances == NULL) {
        printf("Error: malloc for mpi_process_distances failed.\n");
        exit(1);
    }
    double *mpi_process_distances_ptr = (double*)(mpi_process_distances + interval);
    for (i = 0; i < interval; i++) {
        mpi_process_distances[i] = (mpi_process_distances_ptr + nodes_count * i);
        for (j = 0; j < nodes_count; j++) {
            mpi_process_distances[i][j] = distances[start + i][j];
        }        
    }
    
    double **mpi_distances;
    if (rank == 0) {
        mpi_distances = (double**)malloc(sizeof(double*) * (nodes_count-remainder) + sizeof(double) * (nodes_count-remainder) * nodes_count);
        if (mpi_distances == NULL) {
            printf("Error: malloc for mpi_distances failed.\n");
            exit(1);
        }
        double* mpi_distances_ptr = (double*)(mpi_distances + (nodes_count-remainder));
        for (i = 0; i < (nodes_count-remainder); i++) {
            mpi_distances[i] = (mpi_distances_ptr + nodes_count * i);        
        }

    }

    MPI_Gather(*mpi_process_distances, (interval*nodes_count), MPI_DOUBLE, *mpi_distances, (interval*nodes_count), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    free(mpi_process_distances);    
    
    // Handle remaining nodes and finalize program.
    handle_remainders_and_finalize(mpi_distances, rank, remainder);
}

int main(int argc, char **argv)
{
    int rank,size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    // P0 will read Graph Matrix from the input file and send it to rest processes.
    // After initializion completes, every process starts calculating distances.
    if (rank == 0) {
        // Run-time parameters check.
        if (!read_parameters(argv)) {
            printf("Program terminates.\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        fscanf(fin, "%d \n", &nodes_count); // Retrieve Graph nodes count.
        if (nodes_count > 0) {
            printf("Nodes count: %d\n", nodes_count);
            printf("Algorithm started, please wait...\n");    
            initialize_matrix();
            // Broadcast Graph nodes count to rest processes.            
            MPI_Bcast(&nodes_count, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            // Broadcast Graph matrix to rest processes.
            MPI_Bcast(*matrix, (nodes_count*nodes_count), MPI_DOUBLE, 0, MPI_COMM_WORLD);
            // Start processing.
            mpi_d_step(rank, size);
            fclose(fin);
            fclose(fout);
        } else {
            printf("File is empty.\n");
            fclose(fin);
            fclose(fout);
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
    } else {
        // Retrieve Graph nodes count in order to allocate memory for retrieving the Graph matrix.
        MPI_Bcast(&nodes_count, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        allocate_matrix();
        // Retrieve Graph matrix by P0.
        MPI_Bcast(*matrix, (nodes_count*nodes_count), MPI_DOUBLE, 0, MPI_COMM_WORLD);
        mpi_d_step(rank, size);
    }        
    MPI_Finalize();
}

