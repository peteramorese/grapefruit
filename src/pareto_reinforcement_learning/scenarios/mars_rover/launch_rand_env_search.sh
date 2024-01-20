n_instances=3
n_trials=20
#data_directory=updated_total_benchmark
conf=0.1

#mkdir -p $data_directory
#mkdir -p logs

# Uniform 
selector=uniform
../../../../build/bin/PRLGridWorldAgentBenchmark --no-plan-data --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml 
