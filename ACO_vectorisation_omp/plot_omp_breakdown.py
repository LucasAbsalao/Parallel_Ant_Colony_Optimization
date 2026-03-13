import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np

threads_list = [1, 2, 4, 8, 12, 16]
avg_fourmis = []
avg_evap = []

# Varredura para ler os arquivos de iteração
for t in threads_list:
    filename = f"tests/iteration_{t}.csv"
    if os.path.exists(filename):
        df = pd.read_csv(filename)
        avg_fourmis.append(df['Temps_Fourmis_ms'].mean())
        avg_evap.append(df['Temps_Evaporation_ms'].mean())
    else:
        print(f"Arquivo {filename} não encontrado!")
        avg_fourmis.append(0)
        avg_evap.append(0)

# Configurar o Gráfico de Barras Agrupadas
x = np.arange(len(threads_list))
width = 0.35

fig, ax = plt.subplots(figsize=(10, 6))
rects1 = ax.bar(x - width/2, avg_fourmis, width, label='Déplacement (Fourmis)', color='#1f77b4', edgecolor='black')
rects2 = ax.bar(x + width/2, avg_evap, width, label='Évaporation', color='#ff7f0e', edgecolor='black')

ax.set_title("Temps moyen par itération selon le nombre de Threads (OpenMP)", fontsize=14)
ax.set_xlabel("Nombre de Threads", fontsize=12)
ax.set_ylabel("Temps moyen (ms)", fontsize=12)
ax.set_xticks(x)
ax.set_xticklabels(threads_list)
ax.legend()
ax.grid(axis='y', linestyle='--', alpha=0.7)

plt.tight_layout()
plt.savefig('graphe_omp_breakdown.png', dpi=300)
plt.show()