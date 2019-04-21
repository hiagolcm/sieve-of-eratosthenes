#include "sequential-version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct SOE sequential_sieve_of_eratosthenes(long int n) { 
    long int p, count = 0, index;
    char prime[n+1]; 
    struct SOE soe;

    memset(prime, '*', sizeof(prime)); 
  
    for (p = 2; p * p <= n; p ++) { 
        if (prime[p] == '*') { 
            for (int i = p * p; i <= n; i += p) {
                prime[i] = '-';
            }
        } 
    } 

    for (p = 3; p <= n; p++) {
       if (prime[p] == '*')  {
           count ++;
       }
    }

    soe.list = (long int*) malloc(sizeof(long int) * count);

    index = 0;
    for (p = 3; p <= n; p++) {
       if (prime[p] == '*')  {
           soe.list[index] = p;
           index++;
       }
    }

    soe.size = count;

    return soe;
} 