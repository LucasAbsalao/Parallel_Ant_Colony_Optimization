import pandas as pd
import matplotlib.pyplot as plt

# 1. Lê o arquivo gerado pelo C++
df = pd.read_csv("ACO_basic/iteration.csv")

# 2. Cria o gráfico
plt.figure(figsize=(10, 6))

# Plota cada coluna no eixo Y, usando a Iteração no eixo X
plt.plot(df["Iteration"], df["Temps_Fourmis_ms"], label="Mover Formigas", marker='o')
plt.plot(df["Iteration"], df["Temps_Evaporation_ms"], label="Evaporação", marker='x')
plt.plot(df["Iteration"], df["Temps_Update_ms"], label="Atualização", marker='s')

# 3. Formata e exibe
plt.title("Tempo de Computação por Etapa da Simulação")
plt.xlabel("Iteração")
plt.ylabel("Tempo Médio (ms)")
plt.legend()
plt.grid(True)
plt.tight_layout()

# plt.savefig("grafico_tempos.png") # Descomente para salvar a imagem
plt.show()