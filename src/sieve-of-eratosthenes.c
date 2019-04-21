#include "sequential-version.h"

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

void process(long int n, int comm_sz, int my_rank);
long int count_primes(char* list, long int length);
long int find_next_multiple_grater_than(long int x, long int y, long int limit);
long int belongs_to_list(long int number);
void mark_multiples(char* list, long int prime, long int first_value, int length, long int* last_multiple);
long get_best_cache_length();

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
    long int number_of_odds, reduced_size, original_reduced_size, value, sqrt_n, i, current_prime, count = 0, numbers_analyzed = 0, start, global_count;
    char *list;
    long int* last_multiples;
    int length_on_cache;
    struct SOE soe;

    // quantidade de armazenamento no cache
    length_on_cache = get_best_cache_length();

    sqrt_n = sqrt(n) + 1;

    // soe é um objeto contendo todos os primos entre 0 e sqrt_n
    soe = sequential_sieve_of_eratosthenes(sqrt_n);

    // o último múltiplo encontrado de cada primo. Usado para otimizaçao
    last_multiples = (long int*) malloc(sizeof(long int) * soe.size);
    for (i = 0; i < soe.size; i++) {
        last_multiples[i] = 0;
    }

    start = (sqrt_n) % 2 == 0 ? sqrt_n + 1 : sqrt_n + 2; // primeiro valor impar a ser processado
    number_of_odds = ceil((n - start + 1) / 2.0 );  // quantidade de valores ímpares a serem processados
    reduced_size = number_of_odds / comm_sz; // quantidade de ímpares para o processador atual
    original_reduced_size = reduced_size; // quantidade de ímpares para cada processador

    if (my_rank == comm_sz - 1) {
        reduced_size += number_of_odds % comm_sz;
    }

    list = (char*) malloc(sizeof(char) * length_on_cache); // lista de chars; '*' represnta primo e '-' não primo

    value = start + my_rank * original_reduced_size * 2; //primeiro valor a ser calculado

    while (numbers_analyzed < reduced_size) {
        memset(list, '*', length_on_cache);
        numbers_analyzed += length_on_cache;

        if (numbers_analyzed > reduced_size) {
            length_on_cache = length_on_cache - (numbers_analyzed - reduced_size);
        }

        for (i = 0; i < soe.size; i++) {
            current_prime = soe.list[i];
            mark_multiples(list, current_prime, value, length_on_cache, &last_multiples[i]);
        }

        count += count_primes(list, length_on_cache);

        value += 2 * length_on_cache;
    }

    if (my_rank == 0) {
        count += soe.size + 1;
    }

    MPI_Reduce(&count, &global_count, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("glbal_count: %ld\n", global_count);
    }
}


// marca todos os múltiplos de 'prime' da lista
// list - lista
// prime - número cujo a divisibilidade será testada
// first_value - valor que representa o número 0 da lista
// length - tamanho da lista
// last_multiple - o último multiplo de prime encontrado. Valor guardado para otimização
void mark_multiples(char* list, long int prime, long int first_value, int length, long int* last_multiple) {
    long int current_value, last_value;
    int index;

    last_value = first_value + 2 * (length - 1);

    current_value = prime * prime;

    if (current_value < first_value) {
        if (*last_multiple == 0) {
            current_value = find_next_multiple_grater_than(prime, first_value, last_value);
        } else {
            current_value = *last_multiple;
        }
    }

    while (current_value <= last_value) {
        index = (current_value - first_value) / 2;
        list[index] = '-';
        current_value += 2 * prime;
    }

    *last_multiple = current_value;
}

// Econtra o próximo múltiplo de x mario ou igual a y e menor que limit
long int find_next_multiple_grater_than(long int x, long int y, long int limit) {
    long int mod = y % x;

    if (mod == 0) {
        return y;
    }

    while ((x - mod) % 2 != 0) {
        y += 2;
        mod = y % x;
    }

    return mod == 0 ? y : y + (x - y % x);
}


// conta todos os primos da list
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


// encontra a quantidade de linhas presente no cache do processador
long get_best_cache_length() {
    long l1_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    long l2_cache_line_size = sysconf(_SC_LEVEL2_CACHE_LINESIZE); 
    long l3_cache_line_size = sysconf(_SC_LEVEL3_CACHE_LINESIZE);

    long l1_cache_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    long l2_cache_size = sysconf(_SC_LEVEL2_CACHE_SIZE);
    long l3_cache_size = sysconf(_SC_LEVEL3_CACHE_SIZE);

    long l1_lines = l1_cache_size / l1_cache_line_size;
    long l2_lines = l2_cache_size / l2_cache_line_size;
    long l3_lines = l3_cache_size / l3_cache_line_size;

    return l1_lines + l2_lines + l3_lines;
}