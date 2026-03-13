import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np

cores = [1, 2, 4, 8, 12]
avg_times = {
    'Fourmis (CPU)': [],
    'Pheromone MPI_Allreduce (Réseau)': [],
    'Evaporation (CPU)': [],
    'Evaporation MPI_Allgather (Réseau)': []
}

# Ler os dados de cada pasta
for core in cores:
    path = f"test_{core}_cores"
    
    # Acumuladores locais
    t_fourmis = 0
    t_pher_comm = 0
    t_evap = 0
    t_evap_comm = 0
    files_read = 0
    
    # Processar todos os iterationX.csv dentro da pasta
    if os.path.exists(path):
        for file in os.listdir(path):
            if file.startswith("iteration") and file.endswith(".csv"):
                df = pd.read_csv(os.path.join(path, file))
                # Pegar a média de tempo por iteração para este arquivo
                t_fourmis += df['Temps_Fourmis_ms'].mean()
                t_pher_comm += df['Temps_Pheromone_Communicator_ms'].mean()
                t_evap += df['Temps_Evaporation_ms'].mean()
                t_evap_comm += df['Temps_Evaporation_Communicator_ms'].mean()
                files_read += 1
                
        if files_read > 0:
            avg_times['Fourmis (CPU)'].append(t_fourmis / files_read)
            avg_times['Pheromone MPI_Allreduce (Réseau)'].append(t_pher_comm / files_read)
            avg_times['Evaporation (CPU)'].append(t_evap / files_read)
            avg_times['Evaporation MPI_Allgather (Réseau)'].append(t_evap_comm / files_read)
        else:
            for key in avg_times: avg_times[key].append(0)
    else:
        for key in avg_times: avg_times[key].append(0)

# Converter para DataFrame para facilitar o plot
df_plot = pd.DataFrame(avg_times, index=cores)

# Plotar Gráfico de Barras Empilhadas
fig, ax = plt.subplots(figsize=(10, 6))

df_plot.plot(kind='bar', stacked=True, ax=ax, colormap='Set1', edgecolor='black')

ax.set_title("Répartition du Temps par Itération (CPU vs Réseau MPI)", fontsize=14)
ax.set_xlabel("Nombre de Processus (Cœurs)", fontsize=12)
ax.set_ylabel("Temps moyen par itération (ms)", fontsize=12)
plt.xticks(rotation=0)
ax.grid(axis='y', linestyle='--', alpha=0.7)

# Colocar a legenda fora do gráfico para não cobrir as barras
plt.legend(title="Tâches", bbox_to_anchor=(1.05, 1), loc='upper left')

plt.tight_layout()
plt.savefig('graphe_communication.png', dpi=300)
plt.show()