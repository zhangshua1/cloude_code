// PenguinBot - Main Body Shell
// Left and right halves, clamshell design
include <config.scad>

module body_half(is_left=true) {
    difference() {
        union() {
            // Main half-shell
            translate([is_left ? -BODY_W/4 : BODY_W/4, 0, BODY_H/2])
                cube([BODY_W/2, BODY_D, BODY_H], center=true);

            // Rounded outer corners
            translate([is_left ? -BODY_W/2 : BODY_W/2, 0, BODY_H/2])
                cylinder(d=BODY_D, h=BODY_H, center=true, $fn=48);

            // Top dome transition (to head)
            translate([0, 0, BODY_H - 10])
                rotate([0, 90, 0])
                cylinder(d=BODY_D, h=BODY_W/2, center=true, $fn=48);
        }

        // Hollow interior
        translate([is_left ? -BODY_W/4 : BODY_W/4, 0, BODY_H/2 + WALL_THICKNESS])
            cube([BODY_W/2 - WALL_THICKNESS*2, BODY_D - WALL_THICKNESS*2, BODY_H], center=true);

        // Battery compartment (bottom)
        translate([0, 0, WALL_THICKNESS + BATTERY_H/2])
            cube([BATTERY_W + 2, BATTERY_D + 2, BATTERY_H + 2], center=true);

        // PCB mounting bosses
        for (y = [-15, 15])
            for (z = [15, 35])
                translate([is_left ? -BODY_W/2 + 6 : BODY_W/2 - 6, y, z])
                    rotate([0, 90, 0])
                    cylinder(d=M2_TAP, h=10, center=true, $fn=16);

        // Wiring pass-through (top)
        translate([0, 0, BODY_H - 15])
            cube([BODY_W/2 + 2, 15, 12], center=true);

        // Charging port cutout (bottom)
        translate([0, BODY_D/2 + 1, 12])
            rotate([90, 0, 0])
            cylinder(d=10, h=5, center=true, $fn=32);

        // Power switch cutout
        translate([is_left ? 15 : -15, -BODY_D/2 - 1, 12])
            rotate([90, 0, 0])
            cube([12, 8, 5], center=true);

        // Screw bosses (M2 self-tapping)
        for (y = [-BODY_D/2 + 8, BODY_D/2 - 8])
            for (z = [8, BODY_H - 8])
                translate([is_left ? -BODY_W/4 : BODY_W/4, y, z])
                    rotate([0, 90, 0])
                    cylinder(d=M2_TAP, h=15, center=true, $fn=16);

        // Ventilation slots (top rear)
        for (i = [0:4])
            translate([is_left ? -8 : 8, -BODY_D/2 + 10 + i*8, BODY_H - 8])
                cube([3, 4, 2], center=true);

        // Hip servo mount (bottom sides)
        for (side = [-1, 1])
            translate([side * (BODY_W/2 + 5), 0, WALL_THICKNESS + 10])
                rotate([90, 0, 90])
                servo_cutout();
    }
}

// Generate both halves
module body_left()  { body_half(true); }
module body_right() { body_half(false); }

// Assembly helper
module body_assembly() {
    color("DimGray") body_left();
    color("Gray") body_right();
}

// Bottom cover
module bottom_cover() {
    difference() {
        cube([BODY_W - 2, BODY_D - 2, 3], center=true);

        // Screw holes
        for (x = [-BODY_W/2 + 8, BODY_W/2 - 8])
            for (y = [-BODY_D/2 + 8, BODY_D/2 - 8])
                translate([x, y, 0])
                    cylinder(d=M2_CLEAR, h=6, center=true, $fn=16);

        // Charging port cutout
        translate([0, BODY_D/2 - 3, 0])
            cylinder(d=10, h=6, center=true, $fn=32);

        // Power switch slot
        translate([15, -BODY_D/2 + 3, 0])
            cube([13, 9, 6], center=true);
    }
}

// Preview
body_left();
// body_right();
// bottom_cover();
