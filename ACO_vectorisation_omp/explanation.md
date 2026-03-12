3. O Perigo Invisível: Race Condition no mark_pheronome
Dê uma olhada no que acontece dentro do loop das formigas:
phen.mark_pheronome( new_pos_ant );

Nessa função, você faz uma atribuição direta no buffer:

C++
m_buffer_pheronome[( i + 1 ) * m_stride + ( j + 1 )][0] = ... // Cálculo baseado nos vizinhos
O que acontece na vida real: Imagine que a Formiga A (Thread 1) e a Formiga B (Thread 2) resolveram andar para a mesma exata célula do mapa ao mesmo tempo (o que é muito comum, já que elas seguem os mesmos caminhos de feromônio).
As duas threads vão tentar escrever o valor no m_buffer_pheronome no mesmo endereço de memória ao mesmo tempo. Isso é um Data Race (Condição de Corrida).

É grave? No seu caso específico, por sorte, a matemática do mark_pheronome usa apenas m_alpha e os vizinhos da célula. Ou seja, as duas formigas vão calcular exatamente o mesmo número e tentar escrever o mesmo número no buffer. O resultado visual e matemático não vai quebrar.
Porém, em C++, escrever na mesma memória sem sincronização é considerado "Comportamento Indefinido" (Undefined Behavior). Se o professor rodar uma ferramenta de análise como o valgrind --tool=helgrind, ele vai apontar um erro.