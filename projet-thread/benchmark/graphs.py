import sys
import pandas as pd
import matplotlib.pyplot as plt
import os

if len(sys.argv) < 1:
    print("Usage: python generate_graph.py <executable_name>")
    sys.exit(1)

exec_name = sys.argv[1]

# paths des csv 
pthread_path = f"benchmark/logs_pthread/log_{exec_name}.csv"
mythread_path = f"benchmark/logs_mythread/log_{exec_name}.csv"

# creer output_dir sil existe pas 
output_dir = f"benchmark/graph_results/{exec_name}"
os.makedirs(output_dir, exist_ok=True)

# charge les csv et detcte leur nombre de colonnes
def load_csv(path):
    df = pd.read_csv(path, header=0)
    df = df.apply(pd.to_numeric)
    if "nb_yield" in df.columns:
        return df, 2
    else:
        return df, 1


try:
    df_pthread, fmt = load_csv(pthread_path)
    df_mythread, _ = load_csv(mythread_path)
except Exception as e:
    print(f"Erreur lors du chargement des fichiers : {e}")
    sys.exit(1)


plt.figure(figsize=(10, 6))

if fmt == 2:
    df1 = df_pthread.groupby("nb_threads")["execution_time_us"].mean().reset_index()
    df2 = df_mythread.groupby("nb_threads")["execution_time_us"].mean().reset_index()
    y_label = "Temps d'exécution (us)"
else:
    df1 = df_pthread.groupby("nb_threads")[df_pthread.columns[-1]].mean().reset_index()
    df2 = df_mythread.groupby("nb_threads")[df_mythread.columns[-1]].mean().reset_index()
    y_label = "Temps d'exécution (us)"

plt.plot(df1["nb_threads"], df1[df1.columns[-1]], 'o-', label="pthread")
plt.plot(df2["nb_threads"], df2[df2.columns[-1]], 'o-', label="mythread")

plt.title(f"Benchmark - {exec_name}")
plt.xlabel("Nombre de threads")
plt.ylabel(y_label)
plt.grid(True)
plt.legend()
plt.savefig(f"{output_dir}/graph_{exec_name}.png")
plt.close()

print(f"Graphe généré : {output_dir}/graph_{exec_name}.png")
