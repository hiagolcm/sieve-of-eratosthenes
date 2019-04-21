#include "sequential-version.h"

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void process(long int n, int comm_sz, int my_rank);
long int count_primes(char* list, long int length);
long int find_next_multiple_grater_than(long int x, long int y, long int limit);
long int belongs_to_list(long int number);
void mark_multiples(char* list, long int prime, long int first_value, int length);


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
    long int number_of_odds, reduced_size, original_reduced_size, value, sqrt_n, i, current_prime, count = 0, numbers_analyzed = 0, start;
    char *list;
    int length_on_cache, current_length;
    struct SOE soe;

    sqrt_n = sqrt(n) + 1;

    soe = sequential_sieve_of_eratosthenes(sqrt_n);

    // represents only odd numbers
    start = (sqrt_n) % 2 == 0 ? sqrt_n + 1 : sqrt_n + 2;
    number_of_odds = ceil((n - start + 1) / 2.0 ); 
    reduced_size = number_of_odds / comm_sz;
    original_reduced_size = reduced_size;

    length_on_cache = 3900000;

    if (my_rank == comm_sz - 1) {
        reduced_size += number_of_odds % comm_sz;
    }

    list = (char*) malloc(sizeof(char) * length_on_cache);

    value = start + my_rank * original_reduced_size * 2;

    while (numbers_analyzed < reduced_size) {
        memset(list, '*', length_on_cache);
        numbers_analyzed += length_on_cache;

        if (numbers_analyzed > reduced_size) {
            length_on_cache = length_on_cache - (numbers_analyzed - reduced_size);
        }

        for (i = 0; i < soe.size; i++) {
            current_prime = soe.list[i];
            mark_multiples(list, current_prime, value, length_on_cache);
        }

        count += count_primes(list, length_on_cache);

        value += 2 * length_on_cache;
    }

    if (my_rank == 0) {
        count += soe.size;
    }

    printf("count: %ld\n", count);
}

void mark_multiples(char* list, long int prime, long int first_value, int length) {
    long int current_value, last_value;
    int index;

    last_value = first_value + 2 * (length - 1);

    current_value = prime * prime;

    if (current_value < first_value) {
        current_value = find_next_multiple_grater_than(prime, first_value, last_value);
    }

    while (current_value <= last_value) {
        index = (current_value - first_value) / 2;
        list[index] = '-';
        current_value += 2 * prime;
    }
}

long int find_next_multiple_grater_than(long int x, long int y, long int limit) {
    long int mod = y % x;

    if (mod == 0) {
        return y;
    }

    while ((x - mod) % 2 != 0 && y <= limit) {
        y += 2;
        mod = y % x;
    }

 
    return mod == 0 ? y : y + (x - y % x);
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

long int belongs_to_list(long int number) {
    return (number + 1) % 6 == 0 || (number - 1) % 6 == 0;
}