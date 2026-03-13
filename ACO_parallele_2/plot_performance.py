import os
import glob
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def get_num_cores(folder):
    return int(folder.split('_')[1].replace('cores', ''))

# ---------------------------------------------------------
# 1. CONVERGENCE PLOT (Food)
# ---------------------------------------------------------
def plot_convergencia(pastas_teste):
    plt.style.use('seaborn-v0_8-darkgrid')
    plt.figure(figsize=(10, 6))

    for pasta in pastas_teste:
        num_cores = get_num_cores(pasta)
        arquivo_core0 = os.path.join(pasta, "core_0.csv")
        if os.path.exists(arquivo_core0):
            df = pd.read_csv(arquivo_core0)
            if 'FoodCount' in df.columns:
                plt.plot(df['Iteration'], df['FoodCount'], label=f'{num_cores} Cores', linewidth=2)

    plt.title("Swarm Convergence: Food vs Iterations", fontsize=14, fontweight='bold')
    plt.xlabel("Iterations", fontsize=12)
    plt.ylabel("Food Units in Nest", fontsize=12)
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    plt.savefig("plot_01_convergence.png", dpi=300, bbox_inches='tight')
    plt.close()
    print("Generated: plot_01_convergence.png")

# ---------------------------------------------------------
# 2. LINE PLOTS PER TEST (Ants and Time over iterations)
# ---------------------------------------------------------
def plot_linhas_dinamicas(pastas_teste):
    plt.style.use('seaborn-v0_8-darkgrid')

    for pasta in pastas_teste:
        num_cores = get_num_cores(pasta)
        csv_files = glob.glob(os.path.join(pasta, "core_*.csv"))
        if not csv_files: continue
        csv_files.sort(key=lambda x: int(os.path.basename(x).replace("core_", "").replace(".csv", "")))
        
        dataframes = [pd.read_csv(f) for f in csv_files]
        labels = [f"Core {i}" for i in range(num_cores)]
        
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True)
        
        # Ants Plot
        for df, label in zip(dataframes, labels):
            ax1.plot(df['Iteration'], df['NumAnts'], label=label, linewidth=1.5)
        ax1.set_title(f"Load Dynamics ({num_cores} Cores): Ants per Core", fontsize=14, fontweight='bold')
        ax1.set_ylabel("Number of Ants")
        ax1.legend(bbox_to_anchor=(1.02, 1), loc='upper left')
        
        # Active Time Plot (with moving average for smoothing)
        window_size = 50
        for df, label in zip(dataframes, labels):
            smoothed_time = df['ActiveTime_ms'].rolling(window=window_size, min_periods=1).mean()
            ax2.plot(df['Iteration'], smoothed_time, label=label, linewidth=1.5)
        ax2.set_title(f"Active Time per Iteration ({num_cores} Cores)", fontsize=14, fontweight='bold')
        ax2.set_xlabel("Iterations")
        ax2.set_ylabel("Average Active Time (ms)")
        ax2.legend(bbox_to_anchor=(1.02, 1), loc='upper left')

        plt.tight_layout()
        plt.savefig(os.path.join(pasta, f"plot_lines_{num_cores}cores.png"), dpi=300, bbox_inches='tight')
        plt.close(fig)
        print(f"Generated: plot_lines_{num_cores}cores.png in folder {pasta}")

# ---------------------------------------------------------
# 3. STACKED BAR PLOT (Idleness and Bottleneck)
# ---------------------------------------------------------
def plot_time_profiling(pastas_teste, dados_tempo):
    plt.style.use('seaborn-v0_8-darkgrid')

    for pasta in pastas_teste:
        num_cores = get_num_cores(pasta)
        csv_files = glob.glob(os.path.join(pasta, "core_*.csv"))
        if not csv_files: continue
        csv_files.sort(key=lambda x: int(os.path.basename(x).replace("core_", "").replace(".csv", "")))
        
        dataframes = [pd.read_csv(f) for f in csv_files]
        min_len = min(len(df) for df in dataframes)
        dataframes = [df.iloc[:min_len] for df in dataframes] 
        
        tempos_ativos = np.array([df['ActiveTime_ms'].values for df in dataframes])
        tempo_iteracao_max = tempos_ativos.max(axis=0) 
        tempo_total_simulacao = tempo_iteracao_max.sum()
        
        dados_tempo[num_cores] = tempo_total_simulacao # Saves for speedup calculation
        
        tempo_ativo_total_por_core = tempos_ativos.sum(axis=1)
        tempos_ociosos = tempo_total_simulacao - tempo_ativo_total_por_core
        
        fig, ax = plt.subplots(figsize=(10, 6))
        cores_labels = [f"Core {i}" for i in range(num_cores)]
        
        ax.bar(cores_labels, tempo_ativo_total_por_core, label='Active Time (Working)', color='#2ca02c')
        ax.bar(cores_labels, tempos_ociosos, bottom=tempo_ativo_total_por_core, label='Idle Time (Waiting at Barrier)', color='#d62728')
        
        ax.set_title(f"Time Profiling - {num_cores} Cores\n(OpenMP Barrier Impact)", fontweight='bold')
        ax.set_ylabel("Total Time (ms)")
        
        # Rotates X-axis labels to fit nicely
        plt.xticks(rotation=45, ha='right')
        
        # Legend strictly outside the plot
        ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        
        plt.tight_layout()
        plt.savefig(os.path.join(pasta, f"plot_bar_profiling_{num_cores}cores.png"), dpi=300, bbox_inches='tight')
        plt.close(fig)
        print(f"Generated: plot_bar_profiling_{num_cores}cores.png in folder {pasta}")

# ---------------------------------------------------------
# 4. SPEEDUP AND EFFICIENCY PLOTS
# ---------------------------------------------------------
def plot_speedup_eficiencia(dados_tempo):
    if len(dados_tempo) < 2: return

    cores_list = sorted(list(dados_tempo.keys()))
    base_cores = cores_list[0]
    t1 = dados_tempo[base_cores] * base_cores 
    if 1 in dados_tempo: t1 = dados_tempo[1]

    speedup_list = [t1 / dados_tempo[p] for p in cores_list]
    efficiency_list = [speedup_list[i] / cores_list[i] for i in range(len(cores_list))]

    plt.style.use('seaborn-v0_8-darkgrid')
    fig, (ax_s, ax_e) = plt.subplots(1, 2, figsize=(14, 5))
    
    ax_s.plot(cores_list, speedup_list, marker='o', color='blue', linewidth=2, label='Measured Speedup')
    ax_s.plot(cores_list, cores_list, linestyle='--', color='gray', label='Ideal Speedup')
    ax_s.set_title("Speedup", fontweight='bold')
    ax_s.set_xlabel("Number of Threads (Cores)")
    ax_s.set_ylabel("Speedup ($T_1 / T_p$)")
    ax_s.set_xticks(cores_list)
    ax_s.legend()

    ax_e.plot(cores_list, efficiency_list, marker='s', color='purple', linewidth=2)
    ax_e.axhline(y=1.0, linestyle='--', color='gray', label='100% Efficiency')
    ax_e.set_title("Parallel Efficiency", fontweight='bold')
    ax_e.set_xlabel("Number of Threads (Cores)")
    ax_e.set_ylabel("Efficiency (Speedup / Threads)")
    ax_e.set_xticks(cores_list)
    ax_e.set_ylim(0, 1.1)
    ax_e.legend()

    plt.tight_layout()
    plt.savefig("plot_04_speedup_efficiency.png", dpi=300, bbox_inches='tight')
    plt.close()
    print("Generated: plot_04_speedup_efficiency.png")

if __name__ == "__main__":
    pastas_teste = glob.glob("teste_*cores_*")
    if not pastas_teste:
        print("No folder found. Run the C++ simulation first.")
    else:
        pastas_teste.sort(key=get_num_cores)
        dados_tempo = {}
        plot_convergencia(pastas_teste)
        plot_linhas_dinamicas(pastas_teste)
        plot_time_profiling(pastas_teste, dados_tempo)
        plot_speedup_eficiencia(dados_tempo)
        print("\nAll plots generated successfully!")