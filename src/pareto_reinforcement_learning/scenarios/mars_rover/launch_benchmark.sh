n_instances=300
n_trials=40
data_directory=rand_env_benchmark_pev_close
conf=0.1
tag=pev_close

mkdir -p $data_directory
mkdir -p logs

# Uniform 
selector=uniform
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --efe-samples 100 --selector "$selector" -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var_pev_close.yaml -d $data_directory/"$selector"_"$tag".yaml > logs/"$selector"_"$tag".log 2>&1 &

# TOPSIS 
selector=topsis
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --efe-samples 100 --selector "$selector" -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var_pev_close.yaml -d $data_directory/"$selector"_"$tag".yaml > logs/"$selector"_"$tag".log 2>&1 &

# Weights
selector=weights
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --efe-samples 100 --selector "$selector" -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var_pev_close.yaml -d $data_directory/"$selector"_"$tag".yaml > logs/"$selector"_"$tag".log 2>&1 &

# AIF big_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --efe-samples 100 --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var_pev_close.yaml -d $data_directory/aif_big_var_"$tag".yaml > logs/aif_big_var_"$tag".log 2>&1 &

# AIF med_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --efe-samples 100 --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_med_var_pev_close.yaml -d $data_directory/aif_med_var_"$tag".yaml > logs/aif_med_var_"$tag".log 2>&1 &

# AIF small_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --efe-samples 100 --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_small_var_pev_close.yaml -d $data_directory/aif_small_var_"$tag".yaml > logs/aif_small_var_"$tag".log 2>&1 &

# AIF no_var
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentBenchmark --efe-samples 100 --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_no_var_pev_close.yaml -d $data_directory/aif_no_var_"$tag".yaml > logs/aif_no_var_"$tag".log 2>&1 &

