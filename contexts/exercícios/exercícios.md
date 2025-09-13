### 1 - Explique o objetivo e os parâmetros de cada uma das quatro funções acima

A função `getcontext(&a)` tem o objetivo de capturar o contexto atual de execução do programa e armazenar ele em uma variável, que é seu único parâmetro. Usado na `main` para armazenar em `ContextMain`, `ContextPing` e `ContextPong`.

`setcontext(&a)` restaura um contexto previamente setado na variável `a`, não é usado no código.

`swapcontext(&a, &b)` pega o contexto atual do programa e armazena em `a`, e altera o contexto do programa para o contexto salvo em `b`. Usada dentro dos loops das funções `Body(Ping/Pong)`, assim uma fica chamando a outra até os loops acabarem, e aí usa uma última vez para printar o fim de pong, que está fora de seu loop e não tinha sido executado.

`makecontext(&a, …)` define parâmetros para a execução de um contexto ao ser invocado. No programa de exemplo, ela define qual função será chamada quando o contexto for o da variável `a` (`BodyPing` e `BodyPong`), um número que indica quantos argumentos vão ser passados para essa função (`1`) e os argumentos em seguida: `"    Ping:"` e `"    Pong:"`.

### 2 - Explique o significado dos campos da estrutura ucontext_t que foram utilizados no código.

Os três primeiros parâmetros se referem a uc_stack, que em si é a pilha de execução daquele contexto. Nela, são guardadas variáveis locais, parâmetros de funções e endereços de retorno. 

O `uc_stack.ss_sp` basicamente aponta para o bloco de memória que o contexto vai usar, no código ele aponta para o endereço retornado por malloc(STACKSIZE), que é o início do endereço a ser usado.

O `uc_stack_ss_size` armazena o tamanho dessa área de memória alocada para a stack.

o `uc_stack.ss_flags` especifica algumas flags pra comportamento da pilha, no caso do código não recebe nenhuma então é definida como 0.

Já a última, `uc_link`, define um outro contexto para retornar quando o contexto atual terminar sua execução. No caso do código, como a função BodyPing nunca termina, esse campo é mais definido por segurança, porque sempre cai no swap context que manda para a função BodyPong, e o mesmo ocorre com esta.

### 3 - Explique cada linha do código de contexts.c que chame uma dessas funções ou que manipule estruturas do tipo ucontext_t.

-- Expliquei na 1 e na 2 --

### 4 - Para visualizar melhor as trocas de contexto, desenhe o diagrama de tempo dessa execução.

![Diagrama de tempo](./Diagrama%20de%20tempo.png)

![Execução do programa](./Execu%C3%A7%C3%A3o%20do%20programa.png)