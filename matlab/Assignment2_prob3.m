c1 = [0.4003, 0.3988, 0.3998, 0.3997, 0.4010, 0.3995, 0.3991];
c2 = [0.2554, 0.3139, 0.2627, 0.3802, 0.3287, 0.3160, 0.2924];
c3 = [0.5632, 0.7687, 0.0524, 0.7586, 0.4243, 0.5005, 0.6769];

m1 = 0.4; %mean value of the measurement 1
v1 = 0.01; %variance of the measurement 1
m2 = 0.32; %mean value of the measurement 2
v2 = 0.05; %variance of the measurement 2
m3 = 0.55; %mean value of the measurement 3
v3 = 0.2; %variance of the measurement 3

p1 = normpdf(c1, m1, sqrt(v1));
p2 = normpdf(c2, m2, sqrt(v2));
p3 = normpdf(c3, m3, sqrt(v3));

p_meas1 = sum(p1*1/3);
p_meas2 = sum(p2*1/3);
p_meas3 = sum(p3*1/3);

p_joint1 = p1*1/3;
p_joint2 = p2*1/3;
p_joint3 = p3*1/3;

p_pos1 = p_joint1/p_meas1;
p_pos2 = p_joint2/p_meas2;
p_pos3 = p_joint3/p_meas3;