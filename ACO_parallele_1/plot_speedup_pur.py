import os
import pandas as pd
import matplotlib.pyplot as plt

cores = [1, 2, 4, 8, 12]
compute_times = {}

print("Analisando os tempos de cálculo puro...")

# 1. Varredura dos dados por quantidade de processos
for p in cores:
    path = f"test_{p}_cores"
    rank_times = []
    
    if os.path.exists(path):
        for file in os.listdir(path):
            if file.startswith("iteration") and file.endswith(".csv"):
                filepath = os.path.join(path, file)
                df = pd.read_csv(filepath)
                
                # Somar APENAS os tempos de cálculo (CPU) por iteração
                df['Temps_Calcul_Pur'] = (df['Temps_Fourmis_ms'] + 
                                          df['Temps_Evaporation_ms'] + 
                                          df['Temps_Update_ms'])
                
                # Pegar a média desse tempo de cálculo puro para este Rank
                mean_time = df['Temps_Calcul_Pur'].mean()
                rank_times.append(mean_time)
                
    if rank_times:
        # 2. O tempo efetivo de cálculo paralelo é o do processo mais lento (MAX)
        compute_times[p] = max(rank_times)
    else:
        print(f"Aviso: Dados não encontrados para {p} processos.")

# 3. Calcular o Speedup
valid_cores = [p for p in cores if p in compute_times]
valid_times = [compute_times[p] for p in valid_cores]

t_seq = compute_times[1] # Tempo do sequencial
speedups = [t_seq / t for t in valid_times]

# Imprimir os dados no terminal para você colocar na tabela do relatório
print("\n--- Résultats du Speedup (Calcul Pur) ---")
for p, t, s in zip(valid_cores, valid_times, speedups):
    print(f"Cœurs: {p:2d} | Temps CPU moyen: {t:.4f} ms | Speedup: {s:.2f}x")

# 4. Desenhar o Gráfico
plt.figure(figsize=(8, 6))

# Curva do Speedup Isolado (Só CPU)
plt.plot(valid_cores, speedups, marker='o', color='forestgreen', linewidth=2.5, markersize=8, label="Speedup (Calcul Pur)")

# Curva do Speedup Ideal (Linha diagonal y = x)
plt.plot(valid_cores, valid_cores, linestyle='--', color='gray', linewidth=2, label="Speedup Idéal (Linéaire)")

# Configurações visuais do gráfico (Em francês para o seu relatório)
plt.title("Accélération (Speedup) - Uniquement Temps de Calcul", fontsize=15, fontweight='bold', pad=15)
plt.xlabel("Nombre de Processus (Cœurs)", fontsize=12)
plt.ylabel("Accélération", fontsize=12)
plt.xticks(valid_cores)
plt.yticks(range(0, max(valid_cores) + 2, 2))
plt.grid(True, linestyle='--', alpha=0.6)
plt.legend(fontsize=11, loc='upper left')

# Salvar e mostrar
plt.tight_layout()
plt.savefig('graphe_speedup_calcul_pur.png', dpi=300, bbox_inches='tight')
plt.show()