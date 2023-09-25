%%Problem 2a

% y = [ 3 4 7 4 3 5 6 12];
% x = 1:8;
% xq = 1:0.5:8;
% F = interp1(x, y, xq, 'linear');

% figure;
% plot(xq, F, ':.', x, y, 'o');
% xlabel('x');
% ylabel('F');

%%Problem 2c

syms g(x);
g1 = cos(pi/2*x);
g2 = (-pi/2)*( abs(x)^3 -5*(abs(x)^2) + 8*(abs(x)) -4 );
g3 = 0;

g(x) = piecewise(abs(x)<=1, g1, abs(x)>1 & abs(x)<= 2, g2, abs(x)>2, g3);
x = -3:0.05:3;
plot(x, g(x));

x = linspace(1,8,100);
f = [3 4 7 4 3 5 6 12];
Fg = zeros(size(x));
for i = 1:numel(x)
    xq = x(i);
    for j = 1:numel(f)
        Fg(i) = Fg(i) + g(xq-j)*f(j);
    end
end

plot(x, Fg);
