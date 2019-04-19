#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void process(long int n, int comm_sz, int my_rank);
long int get_value_by_index(long int index, long int reduced_size, int my_rank);
void mark_multiples(long int prime, char *list, long int partition_size, int my_rank);
long int count_primes(char* list, long int length);


int main(int argc, char **argv) {
    long int n;
    int comm_sz, my_rank;

    // check if limit superior has been given
    if (argc < 2) {
        printf("limit superior missing.");
        return 0;
    }

    n = strtol(argv[1], NULL, 10);

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    process(n, comm_sz, my_rank);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();


    return 0;
}

long int get_value_by_index(long int index, long int reduced_size, int my_rank) {
    long int k = (index + my_rank * reduced_size) / 2 + 1;
    int suffix = index % 2 == 0 ? -1 : 1;
    return 6 * k + suffix;
}

long int get_index_by_value(long int value) {
    long int k = (value + 1) / 6;
    return (k - 1) * 2;
}

void process(long int n, int comm_sz, int my_rank) {
    long int partition_size, reduced_size, index, value, sqrt_n, count, global_count;
    int i;
    char *list;

    sqrt_n = sqrt(n);

    partition_size = n / comm_sz + 1;
    reduced_size = (partition_size - 1) / 3 + 1;

    list = (char*) malloc(sizeof(char) * reduced_size);
    memset(list, '*', reduced_size);

    if (my_rank == 0) {
        for (index = 0; index < reduced_size; index++) {
            value = get_value_by_index(index, reduced_size, my_rank);
            if (value > sqrt_n) {  break; }

            if (list[index] == '*') {
                for (i = 1; i < comm_sz; i++) {
                    MPI_Send(&value, 1, MPI_LONG_INT, i, 0, MPI_COMM_WORLD);
                }

                mark_multiples(value, list, reduced_size, my_rank);
            }
        }

        value = -1;
        for (i = 1; i < comm_sz; i++) {
            MPI_Send(&value, 1, MPI_LONG_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        do {
            MPI_Recv(&value, 1, MPI_LONG_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (value != -1) {
                mark_multiples(value, list, reduced_size, my_rank);
            }
        } while(value != -1);
    }

    count = count_primes(list, reduced_size);
    printf("count: %ld rank: %d\n", count, my_rank);
    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        //printf("primes: %ld\n", global_count);
    }
}

void mark_multiples(long int prime, char *list, long int reduced_size, int my_rank) {
    long int i;
    long int index_start = get_index_by_value(prime * prime) - my_rank * reduced_size;
    long value;

    if (index_start < 0) {
        index_start = 0;
    }

    for (i = index_start; i < reduced_size; i++) {
        value = get_value_by_index(i, reduced_size, my_rank);

        if (value % prime == 0) {
            list[i] = '-';
        }
    }
}

long int count_primes(char* list, long int length) {
    int i;
    long int count = 0;

    for (i = 0; i < length; i++) {
        if (list[i] == '*') {
            count++;
        }
    }

    return count;
}