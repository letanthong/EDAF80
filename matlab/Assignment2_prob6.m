w1 = [0 0 1; 0 1 0; 0 0 1; 0 1 0; 0 0 1];
w2 = [1 0 1; 0 1 0; 0 1 0; 0 1 0; 1 0 1];
w3 = [1 0 1; 0 1 0; 1 0 1; 0 1 0; 1 0 1];
x = [1 1 1; 0 1 1; 1 0 1; 1 1 0; 0 0 1];
P_b_incorrect = 0.3;
P_w_incorrect = 0.2;
P_b_correct = 0.7;
P_w_correct = 0.8;

P_w1 = 0.35;
P_w2 = 0.4;
P_w3 = 0.25;
P_x_w1 = 1;
P_x_w2 = 1;
P_x_w3 = 1;

for i=1:size(x,1)
    for j = 1:size(x,2)
        if x(i,j) == w1(i,j) && x(i,j) == 0
            P_x_w1 = P_x_w1*P_b_correct;
        elseif x(i,j) == w1(i,j) && x(i,j) == 1
            P_x_w1 = P_x_w1*P_w_correct;
        elseif x(i,j) ~= w1(i,j) && x(i,j) == 0
            P_x_w1 = P_x_w1*P_b_incorrect;
        elseif x(i,j) ~= w1(i,j) && x(i,j) == 1
            P_x_w1 = P_x_w1*P_w_incorrect;
        end
    end
end

for i=1:size(x,1)
    for j = 1:size(x,2)
        if x(i,j) == w2(i,j) && x(i,j) == 0
            P_x_w2 = P_x_w2*P_b_correct;
        elseif x(i,j) == w2(i,j) && x(i,j) == 1
            P_x_w2 = P_x_w2*P_w_correct;
        elseif x(i,j) ~= w2(i,j) && x(i,j) == 0
            P_x_w2 = P_x_w2*P_b_incorrect;
        elseif x(i,j) ~= w2(i,j) && x(i,j) == 1
            P_x_w2 = P_x_w2*P_w_incorrect;
        end
    end
end

for i=1:size(x,1)
    for j = 1:size(x,2)
        if x(i,j) == w3(i,j) && x(i,j) == 0
            P_x_w3 = P_x_w3*P_b_correct;
        elseif x(i,j) == w3(i,j) && x(i,j) == 1
            P_x_w3 = P_x_w3*P_w_correct;
        elseif x(i,j) ~= w3(i,j) && x(i,j) == 0
            P_x_w3 = P_x_w3*P_b_incorrect;
        elseif x(i,j) ~= w3(i,j) && x(i,j) == 1
            P_x_w3 = P_x_w3*P_w_incorrect;
        end
    end
end

P_w1_x = P_x_w1 * P_w1;
P_w2_x = P_x_w2 * P_w2;
P_w3_x = P_x_w3 * P_w3;