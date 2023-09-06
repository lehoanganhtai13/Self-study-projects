clear; clc;
% Validation
x = [];
y = [];
z = [];
tool_length = 150;

for i= 1:370
    if i < 11
        x = [x (20 + 165 - 5) + 5*cos(pi/180)];
        y = [y (0 - 0) + 5*sin(pi/180)];
        z = [z (165 - (40 + tool_length)) - i];
    else 
        x = [x (20 + 165 - 5) + 5*cos((i - 10)*pi/180)];
        y = [y (0 - 0) + 5*sin((i - 10)*pi/180)];
        z = [z (165 - (40 + tool_length)) - 10];
    end
end

plot3(x,y,z,'color','r');
hold on;

% Inverse kinematics
theta1 = [];
theta2 = [];
theta3 = [];
theta4 = [];
theta5 = [];
theta6 = [];

% Current position
Yaw = 180 + (0);
Pitch = 0 + (-20);
Roll = 0 + (0);
Yaw = Yaw*pi/180;
Pitch = Pitch*pi/180;
Roll = Roll*pi/180;

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

for i = 1:370
    Px = x(i);
    Py = y(i);
    Pz = z(i);
    
    % Wrist position
    a1 = 20; a2 = 165; a3 = 0; d4 = -165; d6 = -40;
    d = d6 - tool_length;
    
    PWx = Px - abs(d)*R06(1,3);
    PWy = Py - abs(d)*R06(2,3);
    PWz = Pz - abs(d)*R06(3,3);
    
    a34 = hypot(a3, d4);
    r = hypot(PWx, PWy);
    AB = r - a1;
    len = hypot(PWz, AB);
    
    % Theta1 calculation
    theta1 = [theta1 atan2(PWy, PWx)*180/pi];
    if(theta1(i) < -169 || theta1(i) > 169)
        disp("S-Axis limit is reached");
        return;
    end
    
    % Theta2 calculation
    beta = acos(AB / len);
    alpha = acos((a2^2 + len^2 - a34^2) / (2*a2*len));
    theta2 = [theta2 (pi/2 - alpha - beta)*180/pi];
    if(theta2(i) < -84 || theta2(i) > 89)
        disp("L-Axis limit is reached");
        return;
    end
    
    % Theta3 calculation
    gramma = acos((a2^2 + a34^2 - len^2) / (2*a2*a34));
    theta3 = [theta3 (gramma - pi/2)*180/pi];
    if(theta3(i) < -49 || theta2(i) > 89)
        disp("U-Axis limit is reached");
        return;
    end
    
    % Transformation matrices
    P = [ PWx;
          PWy;
          PWz];
    T06 = [ R06   P;
            0 0 0 1];
    
    % Calculate T01
    syms a1 alpha1 d1
    a1 = 20; alpha1 = -pi/2; d1 = 0;
    a = a1; alpha = alpha1; d = d1; theta = theta1(i)*pi/180;
    T01 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    % Calculate T12
    syms a2 alpha2 d2
    a2 = 165; alpha2 = pi;  d2 = 0;
    a = a2; alpha = alpha2; d = d2; theta = -pi/2 + theta2(i)*pi/180;
    T12 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    % Calculate T23
    syms a3 alpha3 d3
    a3 = 0; alpha3 = -pi/2;  d3 = 0;
    a = a3; alpha = alpha3; d = d3; theta = theta3(i)*pi/180;
    T23 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    T36 = inv(T23)*inv(T12)*inv(T01)*T06;
    
    % Theta5 calculation
    theta5 = [theta5 asin(-T36(3,3))*180/pi];
    if(theta5(i) < -29 || theta5(i) > 109)
        disp("B-Axis limit is reached");
        return;
    end
    
    % Theta4 calculation
    if(cos(theta5*pi/180) > 0)
        theta4 = [theta4 atan2(-T36(2,3),-T36(1,3))*180/pi];
    elseif(cos(theta5*pi/180) < 0)
        theta4 = [theta4 atan2(T36(2,3),T36(1,3))*180/pi];
    end
    if(theta4(i) < -139 || theta4(i) > 139)
        disp("R-Axis limit is reached");
        return;
    end
    
    % Theta6 calculation
    if(cos(theta5*pi/180) > 0)
        theta6 = [theta6 atan2(-T36(3,2),-T36(3,1))*180/pi];
    elseif(cos(theta5*pi/180) < 0)
        theta6 = [theta6 atan2(T36(3,2),T36(3,1))*180/pi];
    end
    if(theta6(i) < -359 || theta2(i) > 359)
        disp("T-Axis limit is reached");
        return;
    end
end

% Forward kinematics
x_f=[];
y_f=[];
z_f=[];
rx_f=[];
ry_f=[];
rz_f=[];
for i = 1:370
    % Calculate T01
    syms a1 alpha1 d1
    a1 = 20; alpha1 = -pi/2; d1 = 0;
    a = a1; alpha = alpha1; d = d1; theta = theta1(i)*pi/180;
    T01 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    % Calculate T12
    syms a2 alpha2 d2
    a2 = 165; alpha2 = pi;  d2 = 0;
    a = a2; alpha = alpha2; d = d2; theta = -pi/2 + theta2(i)*pi/180;
    T12 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    % Calculate T23
    syms a3 alpha3 d3
    a3 = 0; alpha3 = -pi/2;  d3 = 0;
    a = a3; alpha = alpha3; d = d3; theta = theta3(i)*pi/180;
    T23 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    % Calculate T34
    syms a4 alpha4 d4
    a4 = 0; alpha4 = pi/2;  d4 = -165;
    a = a4; alpha = alpha4; d = d4; theta = theta4(i)*pi/180;
    T34 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    % Calculate T45
    syms a5 alpha5 d5
    a5 = 0; alpha5 = -pi/2;  d5 = 0;
    a = a5; alpha = alpha5; d = d5; theta = -pi/2 + theta5(i)*pi/180;
    T45 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    % Calculate T56
    syms a6 alpha6 d6
    a6 = 0; alpha6 = pi;  d6 = -40;
    a = a6; alpha = alpha6; d = d6 - tool_length; theta = theta6(i)*pi/180;
    T56 = [cos(theta)   -sin(theta)*cos(alpha)  sin(theta)*sin(alpha)   a*cos(theta);
           sin(theta)   cos(theta)*cos(alpha)   -cos(theta)*sin(alpha)  a*sin(theta);
           0            sin(alpha)              cos(alpha)              d;
           0            0                       0                       1];
    
    T06 = T01*T12*T23*T34*T45*T56;
     
    x_f = [x_f T06(1,4)];
    y_f = [y_f T06(2,4)];
    z_f = [z_f T06(3,4)];
    ry_f = [ry_f -asin(T06(3,1))*180/pi];
    if(cos(ry_f*pi/180) > 0)
        rx_f = [rx_f atan2(T06(3,2),T06(3,3))*180/pi];
        rz_f = [rz_f atan2(T06(2,1),T06(1,1))*180/pi];
    elseif(cos(ry_f*pi/180) < 0)
        rx_f = [rx_f atan2(-T06(3,2),-T06(3,3))*180/pi];
        rz_f = [rz_f atan2(-T06(2,1),-T06(1,1))*180/pi];
    end
end

plot3(x_f,y_f,z_f,'-o','color','b');
legend('Pattern', 'Validation');
