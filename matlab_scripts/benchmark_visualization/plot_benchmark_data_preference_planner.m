clear; close all; clc;

data = importdata("../../benchmark/benchmark_data/preference_planner_bm.txt");
seg_data.time_lbls = string([]);
seg_data.units = string([]);
seg_data.attr_lbls = string([]);
seg_data.data = {};

for i=1:length(data.data)
    cmd_prefix = "seg_data.data.";
    if ~strcmp(data.textdata(i), ">--")
        field = data.textdata{i};
        if startsWith(field, "-")
            field = formatTimeLbl(field);
            time_lbl = true;
        else
            time_lbl = false;
        end
        if time_lbl
            [inc, inc_i] = ismember(field, seg_data.time_lbls);
        else
            [inc, inc_i] = ismember(field, seg_data.attr_lbls);
        end
        if ~inc
            if time_lbl
                seg_data.time_lbls = [seg_data.time_lbls field];
                inc_i = length(seg_data.time_lbls);
            else
                seg_data.attr_lbls = [seg_data.attr_lbls field];
                inc_i = length(seg_data.attr_lbls);
            end
        end
        if inc_i <= length(seg_data.data)
            if time_lbl
                seg_data.data{1,inc_i} = [seg_data.data{1,inc_i} data.data(i)];
            else
                seg_data.data{2,inc_i} = [seg_data.data{2,inc_i} data.data(i)];
            end
        else
            if time_lbl
                seg_data.data{1,inc_i} = data.data(i);
            else
                seg_data.data{2,inc_i} = data.data(i);
            end

        end
    end
end

A = getByAttr(4, seg_data.data{2}, seg_data.data{1})

function lblf = formatTimeLbl(lbl)
    lblf = erase(lbl, "-");
    lblf = eraseBetween(lblf, "(", ")", "Boundaries","inclusive");
end

function data_arr = getByAttr(attr, attr_data, time_data)
    attr_data == attr
    data_arr = time_data(attr_data == attr);
end


