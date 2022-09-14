function preferenceGridRobotDemo(grid_size, animate, display_prio, use_subplot, max_subplot_length, loi_font_size, show_xyticks)
%%%%%%%%%%%%%%%
% grid_size = 10;
% animate = false;
% display_prio = false;
% use_subplot = true;
% max_subplot_length = 3;
% loi_font_size = 9;
%%%%%%%%%%%%%%%
plan_f = getLines("plan_files/plan.txt");
N_plans = 0;
type = "none";
mu_arr = [];
cost_arr = [];
plan_values = {};
if contains(plan_f{1}, "Type:" ) && contains(plan_f{1}, "single_plan")
    N_plans = 1;
    type = "single_plan";
    use_subplot = false;
elseif contains(plan_f{1}, "Type:" ) && contains(plan_f{1}, "flexibility_plan_list")
    type = "flexibility_plan_list";
    LOI = [];
    for i=2:length(plan_f)
        if (contains(plan_f{i}, "Flexibility"))
            N_plans = N_plans + 1;
            mu_arr(N_plans) = str2double(extractBetween(plan_f{i}, "Flexibility: ",","));
            cost_arr(N_plans) = str2double(extractAfter(plan_f{i}, ", Cost: "));
            j = 0;
            plan_values{N_plans} = strings;
        elseif (contains(plan_f{i}, "LOI"))
            x_str = extractBetween(plan_f{i}, "x", "_");
            y_str = extractBetween(plan_f{i}, "y", "_");
            prio = extractAfter(plan_f{i}, "prio");
            LOI = [LOI; str2double(x_str), str2double(y_str), str2double(prio)];
        else
            j = j + 1;
            plan_values{N_plans}(j) = plan_f{i};
        end

    end
else 
    error("Plan file does not contain type!")
end

if use_subplot
    N_subplots = round(ceil(N_plans/2) * 2);
    subplots_x = N_subplots;
    subplots_y = 1;
    if N_subplots > max_subplot_length
        subplots_x = max_subplot_length;
        subplots_y = ceil(N_subplots/max_subplot_length);
    end
    figure()
    sgtitle("Grid Robot Trajectories")
end

for plan_ind = 1:N_plans
    plan = plan_values{plan_ind};

    %LOI = [];
    %for i=1:length(plan)
    %    if contains(plan{i}, "ap")
    %        if (~end_plan_found)
    %            end_plan_ind = i;
    %            end_plan_found = true;
    %            LOI = zeros(length(plan)-i, 3);
    %        end
    %        x_str = extractBetween(plan{i}, "x", "_");
    %        y_str = extractBetween(plan{i}, "y", "_");
    %        prio = extractAfter(plan{i}, "prio");
    %        LOI(i-end_plan_ind+1, :) = [str2double(x_str), str2double(y_str), str2double(prio)];
    %    end
    %end
    %if ~end_plan_found
    %    end_plan_ind = length(plan)
    %end


    states = zeros(length(plan), 2);
    directions = zeros(length(plan), 4); % x, y, u, v
    arrow_length = .6;
    arrow_grid_offset = .18;
    line_width = 12;
    font_size = 24;
    dot_size = 360;
    text_offset_x = .1;
    text_offset_y = .3;
    states(1,:) = [0, 0]; % Init state



    for i = 1:length(plan)
        if (contains(plan{i}, "move_up"))
            states(i+1, :) = states(i, :) + [0, 1];
            directions(i+1, 1:2) = states(i, :) + [-arrow_grid_offset, 0];
            directions(i+1, 3:4) = arrow_length * [0, 1];

        end
        if (contains(plan{i}, "move_down"))
            states(i+1, :) = states(i, :) + [0, -1];
            directions(i+1, 1:2) = states(i, :) + [arrow_grid_offset, 0];
            directions(i+1, 3:4) = arrow_length * [0, -1];

        end
        if (contains(plan{i}, "move_right"))
            states(i+1, :) = states(i, :) + [1, 0];
            directions(i+1, 1:2) = states(i, :) + [0, arrow_grid_offset];
            directions(i+1, 3:4) = arrow_length * [1, 0];

        end
        if (contains(plan{i}, "move_left"))
            states(i+1, :) = states(i, :) + [-1, 0];
            directions(i+1, 1:2) = states(i, :) + [0, -arrow_grid_offset];
            directions(i+1, 3:4) = arrow_length * [-1, 0];

        end
    end
    states = states + .5;
    directions(:,1:2) = directions(:,1:2) + .5;
   
    if ~isempty(LOI)
        LOI_offset(:,1:2) = LOI(:,1:2) + .5;
    end

    if ~use_subplot
        figure()
    else
        subplot(subplots_y, subplots_x, plan_ind)
    end

    hold on
    % x_vec = 0:grid_size-1;
    % y_vec = 0:grid_size-1;

    scatter(states(1,1), states(1,2), 160, 'r', "filled",'d')
%     text(states(1,1), states(1,2), "Init State")

    axis([0 grid_size 0 grid_size])
    grid on
    if ~show_xyticks
        set(gca,'xticklabel',[])
        set(gca,'yticklabel',[])
    end
    xticks(0:grid_size)
    yticks(0:grid_size)
  
    if type == "single_plan"
        if ~use_subplot
            title("Grid Robot Trajectory",'FontSize',font_size)
        else
            title("Grid Robot Trajectory")
        end
        
    elseif type == "flexibility_plan_list"
        %s = sprintf("$\\mu$ = %.1f, cost = %.1f", mu_arr(plan_ind), cost_arr(plan_ind));
        %title(s,'Interpreter','latex')
        s = sprintf("\\mu = %.1f, cost = %.1f", mu_arr(plan_ind), cost_arr(plan_ind));
        if ~use_subplot
            title(s,'FontSize',font_size)
        else
            title(s)
        end
        
    end
    if ~use_subplot
        xlabel("X",'FontSize',font_size)
        ylabel("Y",'FontSize',font_size)
    else
        xlabel("X")
        ylabel("Y")
    end



    text_array = {[0, 0] , ""};
    if ~isempty(LOI)
        for i=1:length(LOI(:,1))
            I = 0;
            if i~=1
                [~, I] = ismember([LOI(i,1) + text_offset_x, LOI(i,2) + text_offset_y], text_array{1,1}(1:end, 1:2), 'rows');
            end


            if (I == 0) || (i == 1)
                temp_str = sprintf("Prio: %d", LOI(i,3));
                text_array{1,1} = [text_array{1,1}; [LOI_offset(i,1)+text_offset_x, LOI_offset(i,2)+text_offset_y]];
                text_array{1,2} = [text_array{1,2}; temp_str];
            else
                temp_str = sprintf(" & %d", LOI(i,3));
                text_array{1,2}(I) = text_array{1,2}(I) + temp_str;
            end

        end
    end

    % scatter(LOI(:,1), LOI(:,2), 80, "filled", "color", 'r')
    % scatter(states(:,1), states(:,2),40,'filled', "Color",'k')
    if (~animate)
        plot(states(:,1), states(:,2),"LineWidth", line_width, "Color",'c')
        H = quiver(directions(:,1),directions(:,2),directions(:,3),directions(:,4), 0, 'r');
    else
        hold on
        for i=1:length(states(:,1))
            plot(states(1:i,1), states(1:i,2),"LineWidth", 8, "Color",'c')
            H = quiver(directions(1:i,1),directions(1:i,2),directions(1:i,3),directions(1:i,4), 0, 'r');
            pause(.07)
        end
        for i=2:length(text_array{1,1}(:,1))
        text(text_array{1,1}(i,1), text_array{1,1}(i,2), text_array{1,2}(i), "FontSize",12);
        scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, dot_size, "filled", "color",'r')
    end
    end
    for i=2:length(text_array{1,1}(:,1))
        if display_prio
            text(text_array{1,1}(i,1), text_array{1,1}(i,2), text_array{1,2}(i), "FontSize",12);
        end
        scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, dot_size, "filled", "color",'r')
    end
    if plan_ind == 1
        if ~use_subplot
            text(states(1,1), states(1,2), "Init State", 'FontSize',font_size)
        else
            text(states(1,1), states(1,2), "Init State")
        end
    end
end
