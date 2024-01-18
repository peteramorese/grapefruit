n_instances=150
n_trials=100
data_directory=updated_total_benchmark
conf=0.1

mkdir -p $data_directory
mkdir -p logs

# Uniform 
selector=uniform
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/$selector.yaml > logs/$selector.log 2>&1 &

# TOPSIS 
selector=topsis
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/$selector.yaml > logs/$selector.log 2>&1 &

# Weights
selector=weights
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/$selector.yaml > logs/$selector.log 2>&1 &

# AIF big_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/aif_big_var.yaml > logs/aif_big_var.log 2>&1 &

# AIF med_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_med_var.yaml -d $data_directory/aif_med_var.yaml > logs/aif_med_var.log 2>&1 &

# AIF small_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_small_var.yaml -d $data_directory/aif_small_var.yaml > logs/aif_small_var.log 2>&1 &

# AIF no_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_no_var.yaml -d $data_directory/aif_no_var.yaml > logs/aif_no_var.log 2>&1 &

