# sieve-of-eratosthenes

## Compilaçao
Para compilar o projeto basta executar o script compile-project.sh

```
    $ chmod +x compile-project.sh
    $ ./compile-project.sh
```

## Execução

Após a compilação, 2 arquivos ".o" irão surgir na raiz:

* sequential-sieve-of-eratosthenes.o
* sieve-of-eratosthenes.o

Para executa-los basta rodar o comando:

```
    $ ./sequential-sieve-of-eratosthenes.o {n}

    $ mpirun -n {num_processadores} ./sieve-of-eratosthenes.o {n}
```

Onde ```{num_processadores}``` representa o número de processadores e ```{n}``` o límite superior.
