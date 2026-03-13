import pandas as pd
import matplotlib.pyplot as plt

# 1. Carregar os dados da versão sequencial básica
df = pd.read_csv('iteration.csv')

# 2. Configurar a figura com dois subplots (1 linha, 2 colunas)
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# --- GRÁFICO 1: Evolução do tempo ao longo das iterações (Área Empilhada) ---
labels = ['Mise à jour (Update)', 'Évaporation', 'Déplacement (Fourmis)']
colors = ['#2ca02c', '#ff7f0e', '#1f77b4']

ax1.stackplot(df['Iteration'], 
              df['Temps_Update_ms'], 
              df['Temps_Evaporation_ms'], 
              df['Temps_Fourmis_ms'], 
              labels=labels, colors=colors, alpha=0.8)

ax1.set_title("Évolution du temps par itération (Version Séquentielle)", fontsize=13, pad=10)
ax1.set_xlabel("Itération", fontsize=11)
ax1.set_ylabel("Temps d'exécution (ms)", fontsize=11)
ax1.grid(True, linestyle='--', alpha=0.5)
ax1.legend(loc='upper left')

# --- GRÁFICO 2: Proporção média de tempo (Gráfico de Pizza) ---
# Calcula a média de cada coluna
avg_fourmis = df['Temps_Fourmis_ms'].mean()
avg_evap = df['Temps_Evaporation_ms'].mean()
avg_update = df['Temps_Update_ms'].mean()

sizes = [avg_update, avg_evap, avg_fourmis]
explode = (0, 0, 0.05)  # Destacar a fatia das formigas

ax2.pie(sizes, explode=explode, labels=labels, colors=colors, 
        autopct='%1.1f%%', shadow=False, startangle=140, 
        textprops={'fontsize': 11})
ax2.set_title("Répartition du temps de calcul", fontsize=13, pad=10)

# 3. Ajustar e salvar
plt.tight_layout()
plt.savefig('graphe_baseline.png', dpi=300, bbox_inches='tight')
plt.show()