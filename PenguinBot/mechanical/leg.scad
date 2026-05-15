// PenguinBot - Leg Segments
// Upper leg (thigh) + Lower leg (calf) with motor mount
include <config.scad>

// ===== Upper Leg (Thigh) =====
// Connects hip servo to knee servo
module leg_upper(side=1) {  // 1=right, -1=left
    difference() {
        union() {
            // Main body - rounded rectangle
            translate([0, 0, LEG_UPPER_L/2])
                hull() {
                    cylinder(d=10, h=LEG_UPPER_L, center=true, $fn=24);
                    translate([0, 6, 0])
                        cylinder(d=10, h=LEG_UPPER_L, center=true, $fn=24);
                }

            // Hip horn mount (top)
            translate([0, 0, LEG_UPPER_L]) {
                difference() {
                    cylinder(d=18, h=6, $fn=32);
                    // Spline
                    cylinder(d=SERVO_SPLINE_D + CLEARANCE, h=10, center=true, $fn=24);
                }
                // Horn screw holes
                for (a = [0, 120, 240])
                    rotate([0, 0, a])
                        translate([6, 0, 1])
                            cylinder(d=M2_CLEAR, h=8, center=true, $fn=16);
            }

            // Knee servo housing (bottom)
            translate([0, 0, -SERVO_BODY_D/2])
                cube([SERVO_BODY_W + WALL_THICKNESS*2,
                      SERVO_BODY_H + WALL_THICKNESS*2,
                      SERVO_BODY_D], center=true);
        }

        // Knee servo cavity
        translate([0, 0, -SERVO_BODY_D/2])
            servo_cutout();

        // Weight reduction
        translate([0, 0, LEG_UPPER_L/2])
            cube([6, 4, LEG_UPPER_L - 10], center=true);

        // Wire routing channel
        translate([0, 0, 5])
            cylinder(d=4, h=LEG_UPPER_L + 5, center=true, $fn=16);
    }
}

// ===== Lower Leg (Calf) =====
// Connects knee servo to N20 motor + wheel
module leg_lower(side=1) {
    difference() {
        union() {
            // Main body
            translate([0, 0, LEG_LOWER_L/2])
                hull() {
                    cylinder(d=10, h=LEG_LOWER_L, center=true, $fn=24);
                    translate([0, 4, 0])
                        cylinder(d=8, h=LEG_LOWER_L, center=true, $fn=24);
                }

            // Knee horn mount (top)
            translate([0, 0, LEG_LOWER_L]) {
                difference() {
                    cylinder(d=18, h=6, $fn=32);
                    cylinder(d=SERVO_SPLINE_D + CLEARANCE, h=10, center=true, $fn=24);
                }
                for (a = [0, 120, 240])
                    rotate([0, 0, a])
                        translate([6, 0, 1])
                            cylinder(d=M2_CLEAR, h=8, center=true, $fn=16);
            }

            // Motor housing (bottom)
            translate([0, N20_BODY_D/2 + 2, 0])
                rotate([0, 90, 90])
                hull() {
                    cylinder(d=N20_BODY_D + WALL_THICKNESS*2, h=N20_BODY_L + N20_GEARBOX_L + 4, center=true, $fn=32);
                    translate([0, 0, 0])
                        cylinder(d=N20_BODY_D + WALL_THICKNESS*2 + 4, h=N20_BODY_L + N20_GEARBOX_L + 4, center=true, $fn=32);
                }

            // Shaft support bearing boss
            translate([0, -N20_BODY_D/2 - 10, 0])
                cylinder(d=12, h=N20_BODY_L + N20_GEARBOX_L + 4, center=true, $fn=24);

            // Encoder PCB mount bracket
            translate([0, 0, -N20_BODY_L/2 - N20_GEARBOX_L/2 - 2])
                cube([AS5600_PCB_W + 4, 3, AS5600_PCB_H + 4], center=true);
        }

        // Motor cavity
        translate([0, N20_BODY_D/2 + 2, 0])
            rotate([0, 90, 90])
            n20_cutout();

        // Shaft through-hole
        translate([0, 0, 0])
            rotate([0, 90, 0])
            cylinder(d=N20_SHAFT_D + 0.3, h=40, center=true, $fn=16);

        // Encoder PCB slot
        translate([0, 0, -N20_BODY_L/2 - N20_GEARBOX_L/2 - 2])
            cube([AS5600_PCB_W + 0.3, 2, AS5600_PCB_T + 0.3], center=true);

        // Magnet recess
        translate([0, 1.5, 0])
            rotate([90, 0, 0])
            cylinder(d=MAGNET_D + 0.2, h=3, $fn=24);

        // Wire channel
        translate([0, 3, 10])
            cylinder(d=3, h=LEG_LOWER_L + 10, center=true, $fn=16);
    }
}

// ===== Hip Servo Bracket (mounts to body, holds hip servo) =====
module hip_bracket(side=1) {
    difference() {
        union() {
            // Mount plate
            cube([SERVO_BODY_D + 10, 20, 4], center=true);

            // Servo housing
            translate([0, 0, SERVO_BODY_W/2 + 2])
                cube([SERVO_BODY_D + 8, SERVO_BODY_H + WALL_THICKNESS*2, SERVO_BODY_W + 4], center=true);

            // Body mounting flange
            translate([0, side * 15, 0])
                cube([SERVO_BODY_D + 10, 8, 4], center=true);
        }

        // Servo cavity
        translate([0, 0, SERVO_BODY_W/2 + 2])
            rotate([90, 0, 0])
            servo_cutout();

        // Body mount holes
        for (x = [-SERVO_BODY_D/2, SERVO_BODY_D/2])
            translate([x, side * 15, 0])
                cylinder(d=M2_CLEAR, h=10, center=true, $fn=16);
    }
}

// Preview
leg_upper(1);
// leg_lower(1);
// hip_bracket(1);
