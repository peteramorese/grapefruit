n_instances=150
n_trials=1
data_directory=bias_visual_comparison_kld

#conf=1.1

mkdir -p $data_directory
mkdir -p logs


################################ AIF ################################

#conf=0.05
#selector=aif
#tag="0p05"
#var_tag="small_var"
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#var_tag="med_var"
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#var_tag="big_var"
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &

conf=0.1
selector=aif
tag="0p1"
var_tag="small_var"
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
var_tag="med_var"
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
var_tag="big_var"
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &

#conf=0.2
#tag="0p2"
#var_tag="small_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#var_tag="med_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#var_tag="big_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#
#conf=0.3
#tag="0p3"
#var_tag="small_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#var_tag="med_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#var_tag="big_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#
#conf=0.4
#tag="0p4"
#var_tag="small_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#var_tag="med_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &
#var_tag="big_var"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_"$var_tag".yaml -d $data_directory/"$selector"_"$var_tag"_"$tag".yaml --confidence $conf > logs/"$selector"_"$var_tag"_"$tag".log 2>&1 &

################################ UNIFORM & WEIGHTS ################################

#conf=0.05
#selector=uniform
#tag="0p05"
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#selector=weights
#nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &

conf=0.1
selector=uniform
tag="0p1"
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &

selector=weights
nohup stdbuf -oL ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &



#conf=0.2
#selector=uniform
#tag="0p2"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#selector=weights
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#
#conf=0.25
#selector=uniform
#tag="0p25"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#selector=weights
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#
#conf=0.3
#selector=uniform
#tag="0p3"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#selector=weights
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#
#conf=0.35
#selector=uniform
#tag="0p35"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#selector=weights
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#
#conf=0.4
#selector=uniform
#tag="0p4"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#selector=weights
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#
#conf=0.45
#selector=uniform
#tag="0p45"
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#selector=weights
#nohup ../../../../build/bin/PRLGridWorldAgent -r -v --selector $selector -i $n_instances -t 1 -f formulas.yaml -c config_bias_debug_big_var.yaml -d $data_directory/"$selector"_"$tag".yaml --confidence $conf > logs/"$selector"_"$tag".log 2>&1 &
#
#
