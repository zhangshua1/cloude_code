// PenguinBot - Wheel Hub (TPU)
// Flexible tire with D-cut shaft mount
include <config.scad>

module wheel_hub() {
    difference() {
        union() {
            // Tire cylinder
            cylinder(d=WHEEL_DIAMETER, h=WHEEL_WIDTH, center=true, $fn=64);

            // Tread pattern (raised rings)
            for (z = [-WHEEL_WIDTH/2 + 2 : 4 : WHEEL_WIDTH/2 - 2])
                translate([0, 0, z])
                    cylinder(d=WHEEL_DIAMETER + 1.5, h=1.5, center=true, $fn=64);

            // Hub reinforcement
            translate([0, 0, 0])
                cylinder(d=16, h=WHEEL_WIDTH, center=true, $fn=32);
        }

        // D-cut shaft hole
        translate([0, 0, 0])
            union() {
                cylinder(d=N20_SHAFT_D + TIGHT_FIT, h=WHEEL_WIDTH + 2, center=true, $fn=32);
                // Flat
                translate([N20_SHAFT_D/2 - 0.5, 0, 0])
                    cube([N20_SHAFT_D, N20_SHAFT_D + TIGHT_FIT, WHEEL_WIDTH + 2], center=true);
            }
    }
}

// ===== Wheel assembly (with motor representation) =====
module wheel_assembly() {
    // N20 motor body
    color("Silver") cylinder(d=N20_BODY_D, h=N20_BODY_L + N20_GEARBOX_L, center=true, $fn=32);

    // Shaft
    color("Silver")
        translate([0, 0, N20_BODY_L/2 + N20_GEARBOX_L])
            cylinder(d=N20_SHAFT_D, h=N20_SHAFT_L, $fn=24);

    // Wheel
    color("Black")
        translate([0, 0, N20_BODY_L/2 + N20_GEARBOX_L + N20_SHAFT_L/2])
            wheel_hub();
}

// Preview
wheel_hub();
