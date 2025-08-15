#!/bin/bash

# verif des arguments du script
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <executable> <target_time(s)> <lib_used>"
    exit 1
fi

EXECUTABLE=$1
TARGET_TIME=$2
LIB_USED=$3
MAX_EXECS=200


EXEC_NAME=$(basename "$EXECUTABLE")
LOG_FILE_PATH="benchmark/logs_${LIB_USED}/log_${EXEC_NAME}.csv"

# nb de threads avec lesquels tester l'exec
THREAD_COUNTS=(1 2 4 8 16 32 64 128 256 512 1024)

#cree le dir s'il existe pas
if [ ! $("benchmark/logs_${LIB_USED}" -d) ]; 
then
    mkdir benchmark/logs_${LIB_USED}
fi

# cree le fichier si existe pas
if [ ! $($LOG_FILE_PATH -f) ]; 
then
    touch $LOG_FILE_PATH
fi

#change les valeurs sur lesquelles appeler lexec si c'est fibo 
if [ $EXEC_NAME = "51-fibonacci" ];
then 
    THREAD_COUNTS=( 2 4 6 8 10 12 14 16 18 19 20 21 22 )
fi

#change les valeurs sur lesquelles appeler lexec si c'est mutex car il sont lents pour l'instant
if [ $EXEC_NAME = "61-mutex" ]
then 
    THREAD_COUNTS=(1 2 4 8 16 32 64 128)
elif [ $EXEC_NAME = "62-mutex" ];
then
    THREAD_COUNTS=(1 2 4 8 16 32 64)
fi

# reset le fichier de log
echo "nb_threads,execution_time_us" > "$LOG_FILE_PATH"

# foreach thread_count
for nb in "${THREAD_COUNTS[@]}"; 
do
    total_time=0
    count=0

   # while true avec un if break sinon ca marchait pas avec la condition dans le while 
    printf "  Progression %d thread(s) : 0%%" $nb 
    while true; do
        # execute le fichier avec thread_count et récupere le tps
        
        if [ $EXEC_NAME = "51-fibonacci" ];
        then 
            # magouilles parce que fibo retourne pas au même format que les autres
            #echo $("$EXECUTABLE" "$nb")
            execution_time=$("$EXECUTABLE" "$nb" | grep "GRAPH" | awk -F";" '{print $4}')
            #echo $execution_time
            execution_time=$(awk -v t="$execution_time" 'BEGIN { printf "%.0f\n", t * 1000000}')
            #echo $execution_time
        else 
            execution_time=$("$EXECUTABLE" "$nb" | grep "GRAPH" | awk -F";" '{print $4}')
        fi 

        # met à jour total time pour le nb thread courant
        time_to_add=$(echo "$execution_time / 1000000" | bc -l) # bc pour gerer les flottants ( -l = matlib )
        total_time=$(echo "$total_time + $time_to_add" | bc -l)


        # stocke le résultat 
        echo "$nb,$execution_time" >> "$LOG_FILE_PATH"

        # affiche la progression pour cette itération de la boucle for
        echo -en "\r  Progression $nb thread(s) : $(echo "scale=2; $total_time*100/$TARGET_TIME" | bc)%"

        # si target time atteint break 
        if (( $(echo "$total_time >= $TARGET_TIME" | bc -l) )); 
        then
            break
        fi

        # ou si on a dépassé 100 executions
        if [ $count -ge $MAX_EXECS ]; 
        then
            echo -e "\n     Avorté à $count executions"
            break
        fi

        # nouvelle condition: si total_time > 20 secondes
        if (( $(echo "$total_time > 20" | bc -l) )); 
        then
            echo "      Stoppé car le temps total dépasse 20 secondes ##"
            break
        fi

        count=$((count + 1))
    done

    echo "==> $count exécutions avec $nb threads pour atteindre $TARGET_TIME secondes"
done

echo "Benchmark2arg réussit pour $EXECUTABLE,  Résultats stockés dans $LOG_FILE_PATH"
