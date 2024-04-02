n_instances=3
n_trials=300
data_directory=rand_dist_benchmark_c_0p05_test
conf=0.1
tag=rand_dist

mkdir -p $data_directory
mkdir -p logs

selector=uniform
../../../../build/bin/PRLGridWorldAgentBenchmark --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml -c random_config_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --dist-data
#../../../../build/bin/PRLGridWorldAgentRandDistBenchmark --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml --config-filepath config_mars_rover_demo.yaml --random-config-filepath random_config_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --dist-data

# Uniform 
#selector=uniform
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentRandDistBenchmark --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml --config-filepath config_mars_rover_demo.yaml --random-config-filepath random_config_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml > logs/"$selector"_"$tag".log 2>&1 &

## TOPSIS 
#selector=topsis
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentRandDistBenchmark --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml --config-filepath config_mars_rover_demo.yaml --random-config-filepath random_config_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml > logs/"$selector"_"$tag".log 2>&1 &
#
## Weights
#selector=weights
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentRandDistBenchmark --selector $selector -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml --config-filepath config_mars_rover_demo.yaml --random-config-filepath random_config_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml > logs/"$selector"_"$tag".log 2>&1 &
#
## AIF big_var
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentRandDistBenchmark --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml --config-filepath config_mars_rover_demo.yaml --random-config-filepath random_config_big_var.yaml -d $data_directory/aif_big_var_"$tag".yaml > logs/aif_big_var_"$tag".log 2>&1 &
#
## AIF med_var
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentRandDistBenchmark --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml --config-filepath config_mars_rover_demo.yaml --random-config-filepath random_config_med_var.yaml -d $data_directory/aif_med_var_"$tag".yaml > logs/aif_med_var_"$tag".log 2>&1 &
#
## AIF small_var
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentRandDistBenchmark --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml --config-filepath config_mars_rover_demo.yaml --random-config-filepath random_config_small_var.yaml -d $data_directory/aif_small_var_"$tag".yaml > logs/aif_small_var_"$tag".log 2>&1 &
#
## AIF no_var
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgentRandDistBenchmark --selector aif -i $n_instances -t $n_trials --confidence $conf -f formulas.yaml --config-filepath config_mars_rover_demo.yaml --random-config-filepath random_config_no_var.yaml -d $data_directory/aif_no_var_"$tag".yaml > logs/aif_no_var_"$tag".log 2>&1 &
