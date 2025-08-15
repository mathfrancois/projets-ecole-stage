#!/bin/bash

# ce script prend en argument soit nom_executable soit all et lance les benchmarks correspondants 
# sur les executables puis trace les graphes
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <executable|all> <target_time(s)>"
    exit 1
fi

EXEC=$1
TARGET_TIME=$2

# Listes des tests
TEST_PASS_1PARAM=(
    21-create-many
    22-create-many-recursive
    23-create-many-once
    51-fibonacci
    61-mutex
    62-mutex
)

TEST_PASS_2PARAM=(
    31-switch-many
    32-switch-many-join
    33-switch-many-cascade
)

# verifie si les listes d'execs contienent exec 
function contains() {
    local match="$1"
    shift
    for e; do
        [[ "$e" == "$match" ]] && return 0
    done
    return 1
}

# lance le banchmark en fonction du nombre de 
run_benchmark() {
    local exec_name="$1"
    echo "▶Benchmark de $exec_name..."

    if contains "$exec_name" "${TEST_PASS_1PARAM[@]}"; then
        echo "  ➤ Type 1 paramètre"
        ./benchmark/benchmark1arg.sh "build/test/$exec_name" "$TARGET_TIME" "mythread"
        ./benchmark/benchmark1arg.sh "build/test_pthread/$exec_name" "$TARGET_TIME" "pthread"
        python3 benchmark/graphs.py "$exec_name"
    elif contains "$exec_name" "${TEST_PASS_2PARAM[@]}"; then
        echo "  ➤ Type 2 paramètres"
        ./benchmark/benchmark2arg.sh "build/test/$exec_name" "$TARGET_TIME" "mythread"
        ./benchmark/benchmark2arg.sh "build/test_pthread/$exec_name" "$TARGET_TIME" "pthread"
        python3 benchmark/graphs.py "$exec_name"
    else
        echo "Ignoré : $exec_name n’existe pas dans les listes"
    fi
}

# test si on a passé all en arguments
if [[ "$EXEC" == "all" ]]; then
    echo "Lancement de tous les benchmarks..."
    for exec_name in "${TEST_PASS_1PARAM[@]}" "${TEST_PASS_2PARAM[@]}"; do
        run_benchmark "$exec_name"
        echo ""
    done
    echo "✅ Tous les benchmarks sont terminés."
else
    run_benchmark "$EXEC"
fi
