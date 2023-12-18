n_instances=800
n_trials=800

# Uniform 
selector=uniform
nohup ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector $selector -i $n_instances -t $n_trials -f formulas.yaml -c random_config_big_var.yaml -d big_regret_benchmark/$selector.yaml > logs/$selector.log 2>&1 &

# TOPSIS 
selector=topsis
nohup ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector $selector -i $n_instances -t $n_trials -f formulas.yaml -c random_config_big_var.yaml -d big_regret_benchmark/$selector.yaml > logs/$selector.log 2>&1 &

## Weights
#selector=weights
#nohup ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector $selector -i $n_instances -t $n_trials -f formulas.yaml -c random_config_big_var.yaml -d big_regret_benchmark/$selector.yaml > logs/$selector.log 2>&1 &

## AIF big_var
#nohup ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector aif -i $n_instances -t $n_trials -f formulas.yaml -c random_config_big_var.yaml -d big_regret_benchmark/aif_big_var.yaml > logs/aif_big_var.log 2>&1 &
#
## AIF med_var
#nohup ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector aif -i $n_instances -t $n_trials -f formulas.yaml -c random_config_med_var.yaml -d big_regret_benchmark/aif_med_var.yaml > logs/aif_med_var.log 2>&1 &
#
## AIF small_var
#nohup ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector aif -i $n_instances -t $n_trials -f formulas.yaml -c random_config_small_var.yaml -d big_regret_benchmark/aif_small_var.yaml > logs/aif_small_var.log 2>&1 &
#
## AIF no_var
#nohup ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector aif -i $n_instances -t $n_trials -f formulas.yaml -c random_config_no_var.yaml -d big_regret_benchmark/aif_no_var.yaml > logs/aif_no_var.log 2>&1 &
