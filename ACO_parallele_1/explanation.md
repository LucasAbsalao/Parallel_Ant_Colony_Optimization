1. O Pior Inimigo da Vetorização: O while e os if/else (Divergência)
A principal vantagem da vetorização (SoA) é permitir que o compilador use instruções SIMD (Single Instruction, Multiple Data). O SIMD faz a CPU processar 4, 8 ou 16 formigas no mesmo exato pulso de clock.
Mas o SIMD tem uma regra de ouro: todas as formigas no vetor precisam executar exatamente as mesmas instruções.

Olhe para o seu código:

C++
while ( consumed_time < 1. ) { ... }
A formiga idx = 0 pode estar num terreno difícil e o while vai rodar apenas 1 vez. A formiga idx = 1 pode estar num terreno liso e o while vai rodar 3 vezes. Além disso, os vários if/else verificando feromônios fazem cada formiga tomar um caminho diferente no código.
Resultado: O compilador do C++ olha para isso, percebe que é impossível sincronizar as formigas, "desiste" de aplicar o SIMD e executa tudo de forma sequencial de qualquer jeito.

2. O Efeito Colateral do SoA no Cache (Cache Thrashing)
Como o compilador desistiu da vetorização SIMD, a nova estrutura de dados se tornou uma desvantagem para a memória cache da CPU.

Na versão antiga (AoS): A CPU pegava a Formiga[0]. Posição, estado e semente estavam colados na mesma linha de memória RAM. A CPU trazia tudo em 1 única viagem rápida (Cache Hit).

Na versão nova (SoA): Para processar o idx = 0, a CPU tem que buscar o X/Y em pos_ants (Viagem 1), o estado em state_ants (Viagem 2) e a semente em m_seed (Viagem 3). Isso gera muito mais lentidão no acesso à memória (Cache Misses).