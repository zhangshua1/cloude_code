// PenguinBot - Print Configuration & Shared Utilities
// Units: mm | Tolerance for 3D printed parts

// ===== Print Parameters =====
LAYER_HEIGHT = 0.12;  // Appearance parts
WALL_THICKNESS = 1.6; // 3 perimeters @ 0.4mm nozzle → 1.2mm, add safety margin
CLEARANCE = 0.15;     // Hole compensation for FDM
TIGHT_FIT = 0.05;     // Press-fit tolerance
SLIP_FIT = 0.25;      // Sliding fit tolerance

// ===== MG90S Servo Dimensions =====
SERVO_BODY_W = 22.8;
SERVO_BODY_H = 12.2;
SERVO_BODY_D = 29.5;
SERVO_FLANGE_W = 32.2;
SERVO_FLANGE_H = 2.0;
SERVO_MOUNT_HOLE_SPACING = 27.0;
SERVO_MOUNT_HOLE_D = 2.0 + CLEARANCE;
SERVO_SPLINE_D = 4.8;
SERVO_SPLINE_OFFSET = 10.0;  // Spline center from top edge

// ===== N20 Motor Dimensions =====
N20_BODY_D = 24.4;
N20_BODY_L = 13.0;
N20_GEARBOX_D = 24.4;
N20_GEARBOX_L = 12.0;
N20_SHAFT_D = 3.0;
N20_SHAFT_L = 12.0;
N20_MOUNT_HOLE_SPACING = 17.0;
N20_MOUNT_HOLE_D = 2.5;

// ===== AS5600 Encoder =====
AS5600_PCB_W = 16.0;
AS5600_PCB_H = 12.7;
AS5600_PCB_T = 1.6;
MAGNET_D = 6.0;
MAGNET_H = 2.0;

// ===== Display =====
TFT_DISPLAY_W = 31.5;
TFT_DISPLAY_H = 43.5;
TFT_DISPLAY_T = 2.5;
TFT_ACTIVE_W = 24.0;
TFT_ACTIVE_H = 32.0;
TFT_PCB_W = 36.0;
TFT_PCB_H = 52.0;

// ===== Battery 2S 1500mAh (typical) =====
BATTERY_W = 35.0;
BATTERY_H = 18.0;
BATTERY_D = 70.0;

// ===== General Dimensions =====
BODY_W = 60.0;
BODY_D = 45.0;
BODY_H = 55.0;
HEAD_DIAMETER = 55.0;
HEAD_H = 40.0;
LEG_UPPER_L = 28.0;
LEG_LOWER_L = 32.0;
WHEEL_DIAMETER = 45.0;
WHEEL_WIDTH = 12.0;
ROBOT_TOTAL_H = 155.0;

// M2 / M3 fastener clearances
M2_CLEAR = 2.2;
M2_TAP = 1.6;
M2_NUT_D = 4.5;
M2_NUT_H = 1.8;
M3_CLEAR = 3.3;
M3_TAP = 2.5;

// ===== Utility Modules =====
module servo_cutout() {
    // Main body
    translate([0, 0, -SERVO_BODY_D/2])
        cube([SERVO_BODY_W + SLIP_FIT, SERVO_BODY_H + SLIP_FIT, SERVO_BODY_D + 1], center=true);
    // Flange
    translate([0, 0, -SERVO_FLANGE_H/2])
        cube([SERVO_FLANGE_W + SLIP_FIT, SERVO_FLANGE_H + SLIP_FIT, SERVO_BODY_D], center=true);
    // Mounting holes
    for (x = [-1, 1])
        translate([x * SERVO_MOUNT_HOLE_SPACING/2, 0, -SERVO_BODY_D/2])
            cylinder(d=SERVO_MOUNT_HOLE_D, h=SERVO_BODY_D + 2, center=true, $fn=16);
}

module n20_cutout() {
    union() {
        // Motor body
        cylinder(d=N20_BODY_D + SLIP_FIT, h=N20_BODY_L + 1, center=true, $fn=32);
        // Gearbox
        translate([0, 0, N20_BODY_L/2 + N20_GEARBOX_L/2])
            cylinder(d=N20_GEARBOX_D + SLIP_FIT, h=N20_GEARBOX_L + 0.5, center=true, $fn=32);
        // Mount holes
        for (x = [-1, 1])
            translate([x * N20_MOUNT_HOLE_SPACING/2, 0, 0])
                cylinder(d=N20_MOUNT_HOLE_D + CLEARANCE, h=30, center=true, $fn=16);
    }
}

// counterbore for M2 flat head screw
module m2_counterbore(length=10) {
    union() {
        cylinder(d=M2_CLEAR, h=length, center=false, $fn=16);
        translate([0, 0, length - 2.5])
            cylinder(d=4.5, h=3, center=false, $fn=16);
    }
}

// Hex nut trap (horizontal)
module m2_nut_trap() {
    cylinder(d=M2_NUT_D + 0.15, h=M2_NUT_H + 0.1, $fn=6);
}
