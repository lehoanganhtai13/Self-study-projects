clear; clc;
theta1 = -64.1826;
theta2 = 38.7958;
theta3 = -9.29355;
theta4 = 86.9806;
theta5 = 163.909;
theta6 = -72.9008;

Px = 91.77;
Py = -148.653;
Pz = 150.078;
Rx = 156.809;
Ry = 45.7134;
Rz = 10.8643;

tool_length = 72.5;
Rz_offset = -14.1835;

% Calculate T01
syms a1 alpha1 d1
a1 = 20; alpha1 = -pi/2; d1 = 0;
a = a1; alpha = alpha1; d = d1; theta = theta1*pi/180.0;
T01 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

% Calculate T12
syms a2 alpha2 d2
a2 = 165; alpha2 = pi;  d2 = 0;
a = a2; alpha = alpha2; d = d2; theta = -pi/2 + theta2*pi/180.0;
T12 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

% Calculate T23
syms a3 alpha3 d3
a3 = 0; alpha3 = -pi/2;  d3 = 0;
a = a3; alpha = alpha3; d = d3; theta = theta3*pi/180.0;
T23 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

% Calculate T34
syms a4 alpha4 d4
a4 = 0; alpha4 = pi/2;  d4 = -165.0;
a = a4; alpha = alpha4; d = d4; theta = theta4*pi/180;
T34 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

% Calculate T45
syms a5 alpha5 d5
a5 = 0; alpha5 = -pi/2;  d5 = 0;
a = a5; alpha = alpha5; d = d5; theta = -pi/2 + theta5*pi/180;
T45 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

% Calculate T56
syms a6 alpha6 d6
a6 = 0; alpha6 = pi;  d6 = -40.0;
a = a6; alpha = alpha6; d = d6 - tool_length; theta = (theta6 - Rz_offset)*pi/180.0;
T56 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

T06 = T01*T12*T23*T34*T45*T56;

x_f = T06(1,4);
y_f = T06(2,4);
z_f = T06(3,4);

ry_f = -asin(T06(3,1))*180/pi;
if(cos(ry_f*pi/180) > 0)
    rx_f = atan2(T06(3,2),T06(3,3))*180/pi;
    rz_f = atan2(T06(2,1),T06(1,1))*180/pi;
elseif(cos(ry_f*pi/180) < 0)
    rx_f = atan2(-T06(3,2),-T06(3,3))*180/pi;
    rz_f = atan2(-T06(2,1),-T06(1,1))*180/pi;
end

disp("x: " +  x_f);
disp("y: " +  y_f);
disp("z: " +  z_f);
disp("Rx: " +  rx_f);
disp("Ry: " +  ry_f);
disp("Rz: " +  rz_f);