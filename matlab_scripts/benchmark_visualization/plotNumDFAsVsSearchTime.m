%% Plot Number of DFAs vs Search Time Benchmark
% Author: Peter Amorese
% Description: Plots the search time averaged amongst all benchmark data
%   with the same number of formulas agains the number of formulas

function plotNumDFAsVsSearchTime(filepaths, figure_title, legend_entries)
for j = 1:length(filepaths)
    clear seg_data
    data = importdata(filepaths(j));
    seg_data.time_lbls = string([]);
    seg_data.units = string([]);
    seg_data.attr_lbls = string([]);
    seg_data.data = {};

    % Determine the time units:
    for i=1:length(data.textdata)
        if ~strcmp(data.textdata(i), ">--")
            field = data.textdata{i};
            if startsWith(field, "-")
                units = extractBetween(field, "(",")");
                break
            end
        end
    end

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



    dfas_vs_total_time{j} = [];
    dfas_vs_total_time_std{j} = [];

    for i = min(seg_data.data{2}):max(seg_data.data{2})
        temp_data = getByAttr(i, seg_data.data{2}, seg_data.data{1});
        dfas_vs_total_time{j} = [dfas_vs_total_time{j} mean(temp_data)];
        dfas_vs_total_time_std{j} = [dfas_vs_total_time_std{j} std(temp_data)];
    end
%     group_j{j} = min(seg_data.data{2}):max(seg_data.data{2});
end  
%%%%%%%%%%%%%%%%%%%%%%%%%% PLOT HERE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
box_lbls = min(seg_data.data{2}):max(seg_data.data{2});
boxdata = [];
boxdata_std = [];
% group = [];
for j=1:length(filepaths)
    boxdata = [boxdata; dfas_vs_total_time{j}];
    boxdata_std = [boxdata_std; dfas_vs_total_time_std{j}];
%     group = [group; group_j{j}];
end


bar(box_lbls, boxdata)
% boxplot(boxdata, group)
hold on
grid on
% H_std = errorbar(box_lbls, boxdata, -boxdata_std, boxdata_std,'-s','MarkerSize',5, 'MarkerFaceColor','k');
% H_std.Color = 'k';
% H_std.LineStyle = '--';
title(figure_title)
xlabel("Number of Formulas")
ylabel("Total Search Time ("+ units + ")")
legend(legend_entries)

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

end

function lblf = formatTimeLbl(lbl)
    lblf = erase(lbl, "-");
    lblf = eraseBetween(lblf, "(", ")", "Boundaries","inclusive");
end

function data_arr = getByAttr(attr, attr_data, time_data)
    data_arr = time_data(attr_data == attr);
end


