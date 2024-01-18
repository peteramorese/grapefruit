n_instances=150
n_trials=1
data_directory=small_bias_comparison_ucb_edit

#conf=1.1

mkdir -p $data_directory
mkdir -p logs

# Uniform single test 
#selector=uniform
#../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/$selector.yaml --confidence $conf

conf=1
selector=uniform
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_1.yaml --confidence $conf > logs/"$selector"_1.log 2>&1 &

selector=weights
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_01.yaml --confidence $conf > logs/"$selector"_1.log 2>&1 &

conf=1.5
selector=uniform
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_1p5.yaml --confidence $conf > logs/"$selector"_1p5.log 2>&1 &

selector=weights
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_1p5.yaml --confidence $conf > logs/"$selector"_1p5.log 2>&1 &

conf=2
selector=uniform
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_2.yaml --confidence $conf > logs/"$selector"_2.log 2>&1 &

selector=weights
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_2.yaml --confidence $conf > logs/"$selector"_2.log 2>&1 &

conf=2.5
selector=uniform
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_2p5.yaml --confidence $conf > logs/"$selector"_2p5.log 2>&1 &

selector=weights
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_2p5.yaml --confidence $conf > logs/"$selector"_2p5.log 2>&1 &

conf=3
selector=uniform
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_3.yaml --confidence $conf > logs/"$selector"_3.log 2>&1 &

selector=weights
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_3.yaml --confidence $conf > logs/"$selector"_3.log 2>&1 &

conf=3.5
selector=uniform
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_3p5.yaml --confidence $conf > logs/"$selector"_3p5.log 2>&1 &

selector=weights
nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug.yaml -d $data_directory/"$selector"_3p5.yaml --confidence $conf > logs/"$selector"_3p5.log 2>&1 &

