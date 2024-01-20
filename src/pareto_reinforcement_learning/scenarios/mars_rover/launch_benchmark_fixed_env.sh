n_instances=150
n_trials=100
data_directory=fixed_env_benchmark
conf=0.1

mkdir -p $data_directory
mkdir -p logs

######################## few PP ########################
env_tag="few_pp"
seed=11 # 6pp

# Uniform 
selector=uniform
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# TOPSIS 
selector=topsis
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# Weights
selector=weights
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# AIF big_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/aif_big_var_"$env_tag".yaml > logs/aif_big_var_"$env_tag".log 2>&1 &

# AIF med_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_med_var.yaml -d $data_directory/aif_med_var_"$env_tag".yaml > logs/aif_med_var_"$env_tag".log 2>&1 &

# AIF small_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_small_var.yaml -d $data_directory/aif_small_var_"$env_tag".yaml > logs/aif_small_var_"$env_tag".log 2>&1 &

# AIF no_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_no_var.yaml -d $data_directory/aif_no_var_"$env_tag".yaml > logs/aif_no_var_"$env_tag".log 2>&1 &


######################## some PP ########################
env_tag="some_pp"
seed=7 # 32pp

# Uniform 
selector=uniform
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# TOPSIS 
selector=topsis
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# Weights
selector=weights
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# AIF big_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/aif_big_var_"$env_tag".yaml > logs/aif_big_var_"$env_tag".log 2>&1 &

# AIF med_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_med_var.yaml -d $data_directory/aif_med_var_"$env_tag".yaml > logs/aif_med_var_"$env_tag".log 2>&1 &

# AIF small_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_small_var.yaml -d $data_directory/aif_small_var_"$env_tag".yaml > logs/aif_small_var_"$env_tag".log 2>&1 &

# AIF no_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_no_var.yaml -d $data_directory/aif_no_var_"$env_tag".yaml > logs/aif_no_var_"$env_tag".log 2>&1 &

######################## many PP ########################
env_tag="many_pp"
seed=9 # 72pp

# Uniform 
selector=uniform
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# TOPSIS 
selector=topsis
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# Weights
selector=weights
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$env_tag".yaml > logs/"$selector"_"$env_tag".log 2>&1 &

# AIF big_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/aif_big_var_"$env_tag".yaml > logs/aif_big_var_"$env_tag".log 2>&1 &

# AIF med_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_med_var.yaml -d $data_directory/aif_med_var_"$env_tag".yaml > logs/aif_med_var_"$env_tag".log 2>&1 &

# AIF small_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_small_var.yaml -d $data_directory/aif_small_var_"$env_tag".yaml > logs/aif_small_var_"$env_tag".log 2>&1 &

# AIF no_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentFixedEnvBenchmark -s $seed --no-plan-data --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_no_var.yaml -d $data_directory/aif_no_var_"$env_tag".yaml > logs/aif_no_var_"$env_tag".log 2>&1 &

