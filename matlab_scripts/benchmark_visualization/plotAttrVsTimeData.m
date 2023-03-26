%% Plot Flexibility vs Search Time Benchmark
% Author: Peter Amorese
% Description: Plots the search time averaged amongst all benchmark data
%   with the same number of formulas agains the number of formulas
%
% Ex:
%    x_attr_lbl = "flexibility";
%    box_time_data_lbls = ["before_first_search ", "before_total_search "];
function units = plotAttrVsTimeData(filepaths, x_attr_lbl, box_time_data_lbls, group_lbls)
time_attr_marker = "[T] ";
flex_vs_total_time = {};
flex_vs_total_time_std = {};
leg_entries = [];
n_sessions = zeros(size(filepaths));
for j = 1:length(filepaths)
    clear seg_data
    %data = importdata(filepaths(j),':');
    data = manualParse(filepaths(j));
    %data_f = getLines(filepaths(j));
    seg_data.time_lbls = string([]);
    seg_data.units = string([]);
    seg_data.attr_lbls = string([]);
    seg_data.time_data = {};
    seg_data.attr_data = {};
    % Determine the time units:
    for i=1:length(data.textdata)
        if ~strcmp(data.textdata(i), ">--")
            field = data.textdata{i};
            if startsWith(field, time_attr_marker)
                units = extractBetween(field, "(",")");
                break
            end
        end
    end

    for i=1:length(data.data)
        if ~strcmp(data.textdata{i}, ">--")
            field = data.textdata{i};
            if startsWith(field, time_attr_marker)
                field = formatTimeLbl(field, time_attr_marker);
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

            if time_lbl
                if inc_i <= length(seg_data.time_data)
                    seg_data.time_data{inc_i} = [seg_data.time_data{inc_i} data.data(i)];
                else
                    seg_data.time_data{inc_i} = data.data(i);
                end
            else
                if inc_i <= length(seg_data.attr_data)
                    seg_data.attr_data{inc_i} = [seg_data.attr_data{inc_i} data.data(i)];
                else
                    seg_data.attr_data{inc_i} = data.data(i);
                end
            end
        end
    end


    n_sessions(j) = length(data.data(~isnan(data.data)))/(length(seg_data.time_lbls) + length(seg_data.attr_lbls));
    y_vals{j} = [];
    y_vals_std{j} = [];


    box_time_data_inds = [];
    for i=1:length(box_time_data_lbls)
        box_time_data_inds = [box_time_data_inds find(deblank(seg_data.time_lbls)==deblank(box_time_data_lbls(i)), 1)];
    end
    if isempty(box_time_data_inds)
        error("Did not find any benchmarks matching input box time data labels")
    end

    x_attr_ind = find(seg_data.attr_lbls==x_attr_lbl, 1);
    un_attrs = getUniqueAttrs(seg_data.attr_data{x_attr_ind});
    box_time_data{j} = [];
    box_time_data_std{j} = [];
    for i = un_attrs
        for ii = 1:length(box_time_data_inds)
            temp_get_by_attr = getByAttr(i, seg_data.attr_data{x_attr_ind}, seg_data.time_data{box_time_data_inds(ii)});
            temp_data_mean(ii) = mean(temp_get_by_attr);
            temp_data_std(ii) = std(temp_get_by_attr);
        end
        % Arrange in col vector for each time lbl we are plotting:

        box_time_data{j} = [box_time_data{j} temp_data_mean(:)];
        box_time_data_std{j} = [box_time_data_std{j} temp_data_std(:)];
    end
    box_lbls = un_attrs;
    leg_entries = [leg_entries; group_lbls(j) + box_time_data_lbls'];
    %     group_j{j} = min(seg_data.data{2}):max(seg_data.data{2});
end

%%%%%%%%%%%%%%%%%%%%%%%%%% PLOT HERE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



boxdata = [];
boxdata_std = [];
% group = [];
for j=1:length(filepaths)
    boxdata = [boxdata; box_time_data{j}];
    boxdata_std = [boxdata_std; box_time_data_std{j}];
    %     group = [group; group_j{j}];
end


bar(box_lbls, boxdata')
disp(boxdata')

% boxplot(boxdata, group)
hold on
grid on
% H_std = errorbar(box_lbls, boxdata, -boxdata_std, boxdata_std,'-s','MarkerSize',5, 'MarkerFaceColor','k');
% H_std.Color = 'k';
% H_std.LineStyle = '--';
legend(leg_entries,'Interpreter','none')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

end

function lblf = formatTimeLbl(lbl, time_attr_marker)
    lblf = erase(lbl, time_attr_marker);
    lblf = eraseBetween(lblf, "(", ")", "Boundaries","inclusive");
end

function data_arr = getByAttr(attr, attr_data, time_data)
    data_arr = time_data(attr_data == attr);
end

function un_attrs = getUniqueAttrs(attr_data)
    un_attrs = [];
    for i=1:length(attr_data)
        if ~ismember(attr_data(i), un_attrs)
            un_attrs = [un_attrs attr_data(i)];
        end
    end
end

function data = manualParse(filepath)
    %data = importdata(filepaths(j),':');
    data_f = getLines(filepath);
    data.textdata = {};
    data.data = [];
    ind = 1;
    for i=1:length(data_f)
        if contains(data_f{i}, ">--")
            data.textdata{1,ind} = ">--";
            data.data(1,ind) = NaN;
            ind = ind + 1;
        elseif ~contains(data_f{i}, "[Tup]")
            data.textdata{1,ind} = extractBefore(data_f{i}, ":");
            data.data(1,ind) = str2double(extractAfter(data_f{i},":"));
            ind = ind + 1;
        end
    end
end