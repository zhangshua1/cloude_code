// PenguinBot - Full Assembly Preview
// Shows all parts in their proper positions for design verification
include <config.scad>
include <body.scad>
include <head.scad>
include <leg.scad>
include <wheel.scad>

module penguinbot_assembly() {
    // === Body ===
    translate([0, 0, WHEEL_DIAMETER/2 + LEG_LOWER_L + LEG_UPPER_L + 15])
        body_assembly();

    // === Head ===
    translate([0, 0, WHEEL_DIAMETER/2 + LEG_LOWER_L + LEG_UPPER_L + BODY_H + HEAD_DIAMETER/2 - 5]) {
        color("White") head_front();
        color("WhiteSmoke") head_back();
    }

    // === Legs ===
    for (side = [-1, 1]) {
        // Hip bracket
        translate([side * (BODY_W/2 + 5), 0,
                   WHEEL_DIAMETER/2 + LEG_LOWER_L + LEG_UPPER_L + 5])
            color("DimGray") hip_bracket(side);

        // Hip servo
        translate([side * (BODY_W/2 + 10), 0,
                   WHEEL_DIAMETER/2 + LEG_LOWER_L + LEG_UPPER_L + 5])
            rotate([0, 90, 0])
            color("Blue")
                cube([SERVO_BODY_W, SERVO_BODY_H, SERVO_BODY_D], center=true);

        // Upper leg
        translate([side * (BODY_W/2 + 10), 0,
                   WHEEL_DIAMETER/2 + LEG_LOWER_L + LEG_UPPER_L/2])
            color("Gray")
            leg_upper(side);

        // Knee servo
        translate([side * (BODY_W/2 + 15), 0,
                   WHEEL_DIAMETER/2 + LEG_LOWER_L + 5])
            rotate([0, 90, 0])
            color("Blue")
                cube([SERVO_BODY_W, SERVO_BODY_H, SERVO_BODY_D], center=true);

        // Lower leg
        translate([side * (BODY_W/2 + 15), 0,
                   WHEEL_DIAMETER/2 + LEG_LOWER_L/2])
            color("Gray")
            leg_lower(side);

        // Wheel assembly
        translate([side * (BODY_W/2 + 15), 0, WHEEL_DIAMETER/4])
            wheel_assembly();
    }

    // === Display ===
    translate([0, 0, WHEEL_DIAMETER/2 + LEG_LOWER_L + LEG_UPPER_L + BODY_H + HEAD_DIAMETER/2])
        color("#222")
            cube([TFT_DISPLAY_W, TFT_DISPLAY_H, TFT_DISPLAY_T], center=true);

    // === Ground reference ===
    %translate([0, 0, -WHEEL_DIAMETER/2 - 2])
        cube([120, 120, 1], center=true);
}

// Preview full assembly
penguinbot_assembly();

// ===== Dimension Reference =====
echo("===== PenguinBot Dimension Summary =====");
echo("Total Height:", WHEEL_DIAMETER/2 + LEG_LOWER_L + LEG_UPPER_L + BODY_H + HEAD_DIAMETER, "mm");
echo("Total Width:", BODY_W + 30, "mm");
echo("Total Depth:", BODY_D, "mm");
echo("Wheel Diameter:", WHEEL_DIAMETER, "mm");
echo("Estimated CoG Height:", WHEEL_DIAMETER/2 + LEG_LOWER_L + LEG_UPPER_L + BODY_H*0.4, "mm");
