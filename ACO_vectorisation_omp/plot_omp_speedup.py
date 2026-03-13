import pandas as pd
import matplotlib.pyplot as plt

# 1. Carregar os dados atualizados
df = pd.read_csv('tests/resume.csv')

# 2. Calcular o Speedup (Aceleração) com base no Tempo de Computação puro
t_seq = df.loc[df['Threads'] == 1, 'Temps_Computation'].values[0]
df['Speedup'] = t_seq / df['Temps_Computation']

# Imprimir valores para a tabela do relatório
print("--- Résultats du Speedup OpenMP ---")
for index, row in df.iterrows():
    print(f"Threads: {int(row['Threads']):2d} | Temps: {row['Temps_Computation']:.2f} ms | Speedup: {row['Speedup']:.2f}x")

# 3. Configurar a figura
plt.figure(figsize=(9, 6))

# --- GRÁFICO: Speedup (Aceleração) ---
plt.plot(df['Threads'], df['Speedup'], marker='o', color='blue', linewidth=2.5, markersize=8, label="Speedup Mesuré")
plt.plot(df['Threads'], df['Threads'], linestyle='--', color='gray', linewidth=2, label="Speedup Idéal (Linéaire)")

plt.title("Accélération (Speedup) avec OpenMP", fontsize=15, fontweight='bold', pad=15)
plt.xlabel("Nombre de Threads", fontsize=12)
plt.ylabel("Accélération", fontsize=12)
plt.xticks(df['Threads'])
plt.yticks(range(0, int(df['Speedup'].max()) + 3))
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend(fontsize=11)

plt.tight_layout()
plt.savefig('graphe_omp_speedup.png', dpi=300)
plt.show()