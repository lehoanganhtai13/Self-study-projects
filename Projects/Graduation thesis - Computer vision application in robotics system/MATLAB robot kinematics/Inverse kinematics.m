% Clear data
clear; clc;
disp("Robot joints values:");

% Current position
Px = 209.8162;
Py = -185.8333;
Pz = -13.5938;
Yaw = -122.2768;
Pitch = -71.1589;
Roll =  -46.386;
Yaw = Yaw*pi/180;
Pitch = Pitch*pi/180;
Roll = Roll*pi/180;
Rz_offset = 0;

% Rotation matrix for specific pose using Roll-Pitch-Yaw matrix
Rz = [ cos(Roll)      -sin(Roll)  0;
       sin(Roll)      cos(Roll)   0;
       0              0           1];

Ry = [ cos(Pitch)     0           sin(Pitch);
       0              1           0;
       -sin(Pitch)    0           cos(Pitch)];

Rx = [ 1              0           0;
       0              cos(Yaw)    -sin(Yaw);
       0              sin(Yaw)    cos(Yaw)];

R06 = Rz * Ry * Rx;

% Wrist position
a1 = 20; a2 = 165; a3 = 0; d4 = -165; d6 = -40;
tool_length = 72.5;
d6 = d6 - tool_length;

PWx = Px - abs(d6)*R06(1,3);
PWy = Py - abs(d6)*R06(2,3);
PWz = Pz - abs(d6)*R06(3,3);

a34 = hypot(a3, d4);
r = hypot(PWx, PWy);
AB = r - a1;
len = hypot(PWz, AB);

% Theta1 calculation
theta1 = atan2(PWy, PWx);
disp("+Theta1: " + theta1*180/pi);

% Theta2 calculation
if(PWz >= 0)
    beta = acos(AB / len);
elseif(PWz < 0)
    beta = -acos(AB / len);
end
    
alpha = acos((a2^2 + len^2 - a34^2) / (2*a2*len));
theta2 = pi/2 - alpha - beta;
disp("+Theta2: " + theta2*180/pi);

% Theta3 calculation
gramma = acos((a2^2 + a34^2 - len^2) / (2*a2*a34));
% if(gramma < 0)
%     gramma = gramma + 2*pi;
% end
theta3 = gramma - pi/2;
disp("+Theta3: " + theta3*180/pi);

% Transformation matrices
P = [ PWx; 
      PWy; 
      PWz];
T06 = [ R06   P;
        0 0 0 1];

% Calculate T01
syms a1 alpha1 d1
a1 = 20; alpha1 = -pi/2; d1 = 0;
a = a1; alpha = alpha1; d = d1; theta = theta1;
T01 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

% Calculate T12
syms a2 alpha2 d2
a2 = 165; alpha2 = pi;  d2 = 0;
a = a2; alpha = alpha2; d = d2; theta = -pi/2 + theta2;
T12 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

% Calculate T23
syms a3 alpha3 d3
a3 = 0; alpha3 = -pi/2;  d3 = 0;
a = a3; alpha = alpha3; d = d3; theta = theta3;
T23 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
       sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
       0            sin(alpha)              cos(alpha)              d;
       0            0                       0                       1];

T36 = inv(T23)*inv(T12)*inv(T01)*T06;

% Theta5 calculation
theta5 = asin(-T36(3,3));

% Theta4 calculation
if(cos(theta5) > 0)
    theta4 = atan2(-T36(2,3),-T36(1,3));
elseif(cos(theta5) < 0)
    theta4 = atan2(T36(2,3),T36(1,3));
end
disp("+Theta4: " + theta4*180/pi);
disp("+Theta5: " + theta5*180/pi);

% Theta6 calculation
if(cos(theta5) > 0)
    theta6 = atan2(-T36(3,2),-T36(3,1)) + Rz_offset*pi/180.0;
elseif(cos(theta5) < 0)
    theta6 = atan2(T36(3,2),T36(3,1)) + Rz_offset*pi/180.0;
end
disp("+Theta6: " + theta6*180/pi);