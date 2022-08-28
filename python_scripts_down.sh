#! bin/bash
# Convert all python scripts to executables that use the correct container conda environment
chmod -x /root/task_planner/spot_automaton_file_dump/formula2dfa.py
chmod -x /root/task_planner/benchmark/bm_ordered_planner_grid_robot_ndfas.py
chmod -x /root/task_planner/benchmark/bm_ordered_planner_grid_robot_flex.py
chmod -x /root/task_planner/benchmark/bm_ordered_planner_grid_robot_pf.py
chmod -x /root/task_planner/benchmark/bm_preference_planner_grid_robot.py
chmod -x /root/task_planner/test/test_scripts/test_preference_planner_grid_robot_heuristic.py