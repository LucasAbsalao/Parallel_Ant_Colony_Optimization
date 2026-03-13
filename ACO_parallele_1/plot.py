import pandas as pd
import matplotlib.pyplot as plt

# 1. Carregar os dados
df = pd.read_csv('resume.csv')

# 2. Agrupar pelo número de processos (Threads) e pegar o TEMPO MÁXIMO 
# (pois o gargalo é sempre o processo mais lento)
df_grouped = df.groupby('Threads').agg({
    'Temps_Computation': 'max',
    'Temps_Total': 'max'
}).reset_index()

# 3. Calcular o Speedup (Aceleração)
# Formula: S(p) = T(1) / T(p)
t_seq = df_grouped.loc[df_grouped['Threads'] == 1, 'Temps_Computation'].values[0]
df_grouped['Speedup'] = t_seq / df_grouped['Temps_Computation']

# 4. Configurar a figura com 2 gráficos lado a lado
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

# --- GRÁFICO 1: Tempo de Execução ---
ax1.plot(df_grouped['Threads'], df_grouped['Temps_Computation'] / 1000, marker='o', color='red', linewidth=2)
ax1.set_title("Temps de Calcul vs Nombre de Processus", fontsize=14)
ax1.set_xlabel("Nombre de Processus (Cœurs)", fontsize=12)
ax1.set_ylabel("Temps de calcul (Secondes)", fontsize=12)
ax1.grid(True, linestyle='--', alpha=0.7)
ax1.set_xticks(df_grouped['Threads'])

# --- GRÁFICO 2: Speedup (Aceleração) ---
ax2.plot(df_grouped['Threads'], df_grouped['Speedup'], marker='s', color='blue', linewidth=2, label="Speedup Mesuré")
# Linha de speedup ideal (onde S(p) = p)
ax2.plot(df_grouped['Threads'], df_grouped['Threads'], linestyle='--', color='gray', label="Speedup Idéal")
ax2.set_title("Accélération (Speedup)", fontsize=14)
ax2.set_xlabel("Nombre de Processus (Cœurs)", fontsize=12)
ax2.set_ylabel("Accélération", fontsize=12)
ax2.grid(True, linestyle='--', alpha=0.7)
ax2.set_xticks(df_grouped['Threads'])
ax2.legend()

plt.tight_layout()
plt.savefig('graphe_speedup.png', dpi=300)
plt.show()