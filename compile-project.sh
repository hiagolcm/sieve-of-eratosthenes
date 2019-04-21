# compile parallel
cd parallel/src/
mpicc -Wall -lm -g -o ../../sieve-of-eratosthenes.o sieve-of-eratosthenes.c sequential-version.c

# compile sequential
cd ../../sequential/src/
gcc -Wall -lm -g -o ../../sequential-sieve-of-eratosthenes.o sieve-of-eratosthenes.c

