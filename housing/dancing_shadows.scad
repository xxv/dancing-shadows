diameter=100;
strip_width=12;

$fn=100;


smidge=0.05;

/*
difference() {
  cylinder(r1=diameter/2, r2=diameter/2+4, h=strip_width);
  translate([0, 0, -smidge])
    cylinder(r1=diameter/2 -1, r2=diameter/2+3, h=strip_width+smidge*2);
}
*/


for (x = [0 : 1 : 144]) {
  rotate([0, -0.2 * x, 0])
  translate([x * 6, 0, 0])
    pixel();
}

module pixel() {
  h=1.5;
  w=5;
  cube([w,w,h]);
  /*translate([w/2,w/2,h])
    light_cone(100);
    */
}

module light_cone(h=10) {
  angle=160;
  bottom=3;
  color("red", alpha=0.1)
  cylinder(r1=bottom/2, r2=(bottom+tan(angle/2) * h * 2)/2, h=h);

}
