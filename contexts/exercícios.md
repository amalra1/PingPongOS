### 1 - Explique o objetivo e os parâmetros de cada uma das quatro funções acima

**`getcontext`**
    A função `getcontext(&a)` tem o objetivo de capturar o contexto atual de execução do programa e armazenar ele em uma variável, que é seu único parâmetro. Usado na `main` para armazenar em `ContextMain`, `ContextPing` e `ContextPong`.

**`setcontext`**
    `setcontext(&a)` restaura um contexto previamente setado na variável `a`, não é usado no código.

**`swapcontext`**
    `swapcontext(&a, &b)` pega o contexto atual do programa e armazena em `a`, e altera o contexto do programa para o contexto salvo em `b`. Usada dentro dos loops das funções `Body(Ping/Pong)`, assim uma fica chamando a outra até os loops acabarem, e aí usa uma última vez para printar o fim de pong, que está fora de seu loop e não tinha sido executado.

**`makecontext`**
    `makecontext(&a, …)` define parâmetros para a execução de um contexto ao ser invocado. No programa de exemplo, ela define qual função será chamada quando o contexto for o da variável `a` (`BodyPing` e `BodyPong`), um número que indica quantos argumentos vão ser passados para essa função (`1`) e os argumentos em seguida: `"    Ping:"` e `"    Pong:"`.

#### 2 - Explique o significado dos campos da estrutura ucontext_t que foram utilizados no código.

#### 3 - Explique cada linha do código de contexts.c que chame uma dessas funções ou que manipule estruturas do tipo ucontext_t.

#### 4 - Para visualizar melhor as trocas de contexto, desenhe o diagrama de tempo dessa execução.