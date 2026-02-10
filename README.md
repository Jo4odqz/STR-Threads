# Simulação de Célula de Manufatura Automatizada

Este projeto implementa uma simulação concorrente de uma célula de manufatura automatizada utilizando a linguagem C e a biblioteca `pthread`. O sistema coordena duas máquinas de processamento, um robô industrial compartilhado e uma esteira (buffer) de saída com capacidade limitada.

## 📋 Descrição do Problema

O objetivo do sistema é transformar matéria-prima em peças acabadas e depositá-las na esteira de saída sem causar erros operacionais. O desafio principal é coordenar o uso do robô compartilhado para evitar:

* **Colisão de recursos:** O robô tentar acessar duas máquinas ao mesmo tempo.
* **Transbordamento (Overflow):** Depositar peças quando o buffer está cheio.
* **Bloqueio (Deadlock):** O sistema travar porque o buffer está cheio e as máquinas estão bloqueadas esperando o robô.

### Componentes do Sistema
1.  [cite_start]**Máquinas (M1 e M2):** Processam peças independentemente e aguardam retirada.
2.  **Robô:** Agente de transporte unitário. [cite_start]Move peças das máquinas para o Buffer.
3.  [cite_start]**Buffer:** Armazenamento temporário com capacidade limitada a **2 peças**.
4.  [cite_start]**Consumidor (Agente Externo):** Simula a retirada de peças do buffer para liberar espaço.

---

## 🚀 Visão Geral da Solução Implementada

A solução foi desenvolvida utilizando o modelo de **Programação Concorrente** com threads. Cada componente ativo do sistema (Máquina 1, Máquina 2, Robô e Consumidor) é representado por uma thread independente que roda em um loop infinito.

### Estrutura de Dados
Foi utilizada uma estrutura compartilhada chamada `Manufatura` que contém o estado global do sistema, protegida por mecanismos de sincronização para evitar condições de corrida.

### Mecanismo de Sincronização
Para garantir a segurança e a ordem das operações, foram utilizados:
1.  **Mutex (`pthread_mutex_t`):** Um único cadeado protege toda a estrutura `Manufatura`, garantindo exclusão mútua. Apenas uma thread pode alterar o estado do sistema por vez.
2.  **Variáveis de Condição (`pthread_cond_t`):**
    * `condMaquinas`: Faz as máquinas esperarem até que o robô retire a peça.
    * `condRobo`: Faz o robô dormir quando não há peças prontas nas máquinas ou tarefas pendentes.
    * `condBuffer`: Faz o robô dormir se o buffer estiver cheio (evita overflow) e faz o consumidor dormir se o buffer estiver vazio.

### Lógica do Robô (Cérebro do Sistema)
A função do robô opera como uma máquina de estados:
1.  **Se estiver carregando:** Verifica se há espaço no buffer.
    * *Se cheio:* Dorme na variável `condBuffer`.
    * *Se livre:* Deposita a peça, avisa o consumidor e libera as máquinas.
2.  **Se estiver livre:** Verifica se M1 ou M2 têm peças prontas.
    * *Se sim:* Coleta a peça e sinaliza a máquina para iniciar novo ciclo.
    * *Se não:* Dorme na variável `condRobo` até ser acordado por uma máquina.

---

## 🛠️ Como Compilar e Executar

O projeto utiliza a biblioteca POSIX Threads (`pthread`), nativa em sistemas Linux/Unix.

### Pré-requisitos
* Compilador GCC.
* Ambiente Linux ou WSL (Windows Subsystem for Linux).

### Compilação
Abra o terminal na pasta do projeto e execute o comando abaixo. A flag `-pthread` é essencial para vincular a biblioteca de threads.

```bash
gcc prj1.c -o prj1 -pthread
