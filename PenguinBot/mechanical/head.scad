// PenguinBot - Head Shell
// Front (display window) + Back (servo mount) halves
include <config.scad>

module head_front() {
    difference() {
        union() {
            // Main dome
            sphere(d=HEAD_DIAMETER, $fn=64);
            // Neck transition
            translate([0, 0, -HEAD_DIAMETER/2])
                cylinder(d=30, h=12, $fn=32);
        }

        // Hollow
        sphere(d=HEAD_DIAMETER - WALL_THICKNESS*2, $fn=64);

        // Display window cutout
        translate([0, 0, 5])
            cube([TFT_ACTIVE_W - 2, TFT_ACTIVE_H - 2, HEAD_DIAMETER], center=true);

        // Display PCB mounting frame
        translate([0, 0, -HEAD_DIAMETER/2 + 10])
            cube([TFT_PCB_W + 2, TFT_PCB_H + 2, HEAD_DIAMETER/2], center=true);

        // Display PCB screw posts (M2)
        for (x = [-TFT_PCB_W/2 + 3, TFT_PCB_W/2 - 3])
            for (z = [-TFT_PCB_H/2 + 3, TFT_PCB_H/2 - 3])
                translate([x, 2, z + 5])
                    rotate([90, 0, 0])
                    m2_counterbore(6);

        // Split plane (front half)
        translate([0, HEAD_DIAMETER/2, 0])
            cube([HEAD_DIAMETER + 2, HEAD_DIAMETER, HEAD_DIAMETER + 2], center=true);

        // Snap-fit tabs (female)
        for (a = [45, 135, 225, 315])
            rotate([0, 0, a])
                translate([0, HEAD_DIAMETER/2 - 8, 0])
                    cube([8, 4, 3], center=true);
    }
}

module head_back() {
    difference() {
        union() {
            // Main dome
            sphere(d=HEAD_DIAMETER, $fn=64);
            // Neck transition
            translate([0, 0, -HEAD_DIAMETER/2])
                cylinder(d=30, h=12, $fn=32);
        }

        // Hollow
        sphere(d=HEAD_DIAMETER - WALL_THICKNESS*2, $fn=64);

        // Split plane (back half)
        translate([0, -HEAD_DIAMETER/2, 0])
            cube([HEAD_DIAMETER + 2, HEAD_DIAMETER, HEAD_DIAMETER + 2], center=true);

        // Servo mount (neck yaw)
        translate([0, -HEAD_DIAMETER/2 + 8, -HEAD_DIAMETER/2 + 5])
            rotate([0, -90, 0])
            servo_cutout();

        // Wiring hole to body
        translate([0, -HEAD_DIAMETER/2 + 12, -HEAD_DIAMETER/2])
            cylinder(d=12, h=15, $fn=32);

        // Vent slots
        for (i = [0:3])
            translate([-10 + i*7, -HEAD_DIAMETER/2 + 5, HEAD_DIAMETER/2 - 5])
                cube([2, 8, 3], center=true);

        // Snap-fit sockets
        for (a = [45, 135, 225, 315])
            rotate([0, 0, a])
                translate([0, -HEAD_DIAMETER/2 + 8, 0])
                    cube([8.2, 4.2, 3.2], center=true);
    }
}

// Neck servo mounting bracket
module neck_servo_bracket() {
    difference() {
        union() {
            cylinder(d=28, h=6, $fn=32);
            // Horn mount
            translate([0, 0, 3])
                cylinder(d=14, h=8, $fn=32);
        }
        // Center bore for wires
        cylinder(d=8, h=20, center=true, $fn=24);
        // Servo horn screw holes
        for (a = [0, 120, 240])
            rotate([0, 0, a])
                translate([5, 0, 0])
                    cylinder(d=M2_CLEAR, h=10, center=true, $fn=16);
    }
}

// Preview
head_front();
// head_back();
// neck_servo_bracket();
