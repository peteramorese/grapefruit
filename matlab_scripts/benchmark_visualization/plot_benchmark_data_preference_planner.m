clear; close all; clc;

data = importdata("../../benchmark/benchmark_data/preference_planner_bm.txt");
seg_data.time_lbls = [];
seg_data.attr_lbls = [];

for i=1:length(data.textdata)
    str = data.textdata{i};
    if startsWith(str, "-")
        units = extractBetween(str,"(",")");
        if units == "us"
            units = '{\mu}s';
        end
        lbl = erase(str, "-");
        lbl = er
        cell = {erase(str, "-"), units};
        seg_data.time_lbls = [seg_data.time_lbls cell];
    elseif strcmp(str, ">--")
        break
    else
        seg_data.attr_lbls = [seg_data.attr_lbls str];
    end
end
for i=1:length(data.data)
    cmd_prefix = "seg_data.data.";
    if ~strcmp(data.textdata(i), ">--")
        field = data.textdata{i};
        if startsWith(field, "-")
            field = erase(field, "-");
            field = eraseBetween(field, "(", ")", "Boundaries","inclusive");
        end
        try
            cmd = cmd_prefix + field + "= [" + cmd_prefix + field + ", " + string(data.data(i)) + "];";
            eval(cmd)
        catch
            cmd = cmd_prefix + field + "=" + string(data.data(i)) + ";";
            eval(cmd)
        end
    end
end


