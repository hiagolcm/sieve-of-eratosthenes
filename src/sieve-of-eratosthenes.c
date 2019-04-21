#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void process(long int n, int comm_sz, int my_rank);
void mark_multiples(long int prime, char *list, long int partition_size, int my_rank);
long int count_primes(char* list, long int length, int my_rank, int comm_sz, long int limit);
long int find_next_multiple_grater_than(long int x, long int y);


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

void process(long int n, int comm_sz, int my_rank) {
    long int reduced_size, index, value, sqrt_n, count;
    int i;
    char *list;

    sqrt_n = sqrt(n);

    // represents only odd numbers
    reduced_size = (n - 1) / (comm_sz * 2) + 1; 

    // instatiate list. '*' will represt a prime number and '-' a non-prime.
    list = (char*) malloc(sizeof(char) * reduced_size);
    memset(list, '*', reduced_size);

    if (my_rank == 0) {
        value = 3;
        for (index = 0; index < reduced_size; index++) {
            if (value > sqrt_n) {  break; }

            if (list[index] == '*') {
                for (i = 1; i < comm_sz; i++) {
                    MPI_Send(&value, 1, MPI_LONG_INT, i, 0, MPI_COMM_WORLD);
                }

                mark_multiples(value, list, reduced_size, my_rank);
            }

            value += 2;
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

    count = count_primes(list, reduced_size, my_rank, comm_sz, (n - 1) / 2);
    printf("count: %ld rank: %d\n", count, my_rank);
    // MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        //printf("primes: %ld\n", global_count);
    }
}

void mark_multiples(long int prime, char *list, long int reduced_size, int my_rank) {
    long int current_value, last_value, first_value, index;

    first_value = my_rank * reduced_size * 2 + 3;
    last_value = first_value + (reduced_size - 1) * 2;

    current_value = prime * prime;

    if (current_value < first_value) {
        current_value = find_next_multiple_grater_than(prime, first_value);
    }

    while (current_value <= last_value) {
        index = current_value / 2 - 1 - my_rank * reduced_size;
        list[index] = '-';
        current_value += 2 * prime;
    }
}

long int find_next_multiple_grater_than(long int x, long int y) {
    while (y % x != 0) {
        y += 2;
    }

    return y;
}

long int count_primes(char* list, long int length, int my_rank, int comm_sz, long int limit) {
    int i;
    long int count = 0, index;

    for (i = 0; i < length; i++) {
        if (my_rank == comm_sz - 1) {
            index = my_rank * length + i;

            if (index >= limit - 1) {
                break;
            }
        }

        if (list[i] == '*') {
            count++;
        }
    }

    return count;
}