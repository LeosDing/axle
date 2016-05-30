#include "shapes.h"
#include "gfx.h"
#include <std/math.h>

//convenience functions to make life easier
double line_length(Line line) {
	//distance formula
	return sqrt(pow(line.p2.x - line.p1.x, 2) + pow(line.p2.y - line.p1.y, 2));
}

Coordinate line_center(Line line) {
	//average coordinates together
	double x = (line.p1.x + line.p2.x) / 2;
	double y = (line.p1.y + line.p2.y) / 2;
	return create_coordinate(x, y);
}

Coordinate triangle_center(Triangle t) {
	//average coordinates together
	double x = (t.p1.x + t.p2.x + t.p3.x) / 3;
	double y = (t.p1.y + t.p2.y + t.p3.y) / 3;
	return create_coordinate(x, y);
}

//functions to create shape structures
Coordinate create_coordinate(int x, int y) {
	Coordinate coord;
	coord.x = x;
	coord.y = y;
	return coord;
}

Size create_size(int w, int h) {
	Size size;
	size.width = w;
	size.height = h;
	return size;
}

Rect create_rect(Coordinate origin, Size size) {
	Rect rect;
	rect.origin = origin;
	rect.size = size;
	return rect;
}

Line create_line(Coordinate p1, Coordinate p2) {
	Line line;
	line.p1 = p1;
	line.p2 = p2;
	return line;
}

Circle create_circle(Coordinate center, int radius) {
	Circle circle;
	circle.center = center;
	circle.radius = radius;
	return circle;
}

Triangle create_triangle(Coordinate p1, Coordinate p2, Coordinate p3) {
	Triangle triangle;
	triangle.p1 = p1;
	triangle.p2 = p2;
	triangle.p3 = p3;
	return triangle;
}

//functions to draw shape structures
static void draw_rect_int_fast(Screen* screen, Rect rect, int color) {
	for (int y = rect.origin.y; y < rect.origin.y + rect.size.height; y++) {
		for (int x = rect.origin.x; x < rect.origin.x + rect.size.width; x++) {
			putpixel(screen, x, y, color);
		}
	}
}

static void draw_rect_int(Screen* screen, Rect rect, int color) {
	Line h1 = create_line(rect.origin, create_coordinate(rect.origin.x + rect.size.width, rect.origin.y));
	Line h2 = create_line(create_coordinate(rect.origin.x, rect.origin.y + rect.size.height), create_coordinate(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height));
	Line v1 = create_line(rect.origin, create_coordinate(rect.origin.x, rect.origin.y + rect.size.height));
	Line v2 = create_line(create_coordinate(rect.origin.x + rect.size.width, rect.origin.y), create_coordinate(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height + 1));

	draw_line(screen, h1, color, 1);
	draw_line(screen, h2, color, 1);
	draw_line(screen, v1, color, 1);
	draw_line(screen, v2, color, 1);
}

void draw_rect(Screen* screen, Rect r, int color, int thickness) {
	int max_thickness = (MIN(r.size.width, r.size.height)) / 2;

	//if thickness is negative, fill the shape
	if (thickness < 0) thickness = max_thickness;
	
	//make sure they don't request a thickness too big
	thickness = MIN(thickness, max_thickness);

	//a filled shape is a special case that can be drawn faster
	if (thickness == max_thickness) {
		draw_rect_int_fast(screen, r, color);
		return;
	}

	int x = r.origin.x;
	int y = r.origin.y;
	int w = r.size.width;
	int h = r.size.height;
	for (int i = 0; i <= thickness; i++) {
		Coordinate origin = create_coordinate(x, y);
		Size size = create_size(w, h);
		Rect rt = create_rect(origin, size);

		draw_rect_int(screen, rt, color);

		//decrement values for next shell
		x++;
		y++;
		w -= 2;
		h -= 2;
	}
}

void draw_hline_fast(Screen* screen, Line line, int color, int thickness) {
	for (int i = 0; i < thickness; i++) {
		//calculate starting point
		//increment y for next thickness since this line is horizontal
		int loc = (line.p1.x * screen->depth / 8) + ((line.p1.y + i) * (screen->depth / 8));
		for (int j = 0; j < (line.p2.x - line.p1.x); j++) {
			/*
			screen->physbase[loc + 0] = color & 0xFF; //blue
			screen->physbase[loc + 1] = (color >> 8) & 0xFF; //green
			screen->physbase[loc + 2] = (color >> 16) & 0xFF; //red
			*/
		}
	}
}

void draw_vline_fast(Screen* screen, Line line, int color, int thickness) {
	for (int i = 0; i < thickness; i++) {
		//calculate starting point
		//increment x for next thickness since line is vertical
		uint16_t loc = (line.p1.y * screen->window->size.width) + (line.p1.x + i);
		for (int j = 0; j < (line.p2.y - line.p1.y); j++) {
			screen->vmem[loc + (j * screen->window->size.width)] = color;	
		}
		
	}
}

void draw_line(Screen* screen, Line line, int color, int thickness) {
	//if the line is perfectly vertical or horizontal, this is a special case
	//that can be drawn much faster
	/*
	if (line.p1.x == line.p2.x) {
		draw_vline_fast(screen, line, color, thickness);
		return;
	}
	else if (line.p1.y == line.p2.y) {
		draw_hline_fast(screen, line, color, thickness);
		return;
	}
	*/
	
	int t;
	int distance;
	int xerr = 0, yerr = 0, delta_x, delta_y;
	int incx, incy;

	//get relative distances in both directions
	delta_x = line.p2.x - line.p1.x;
	delta_y = line.p2.y - line.p1.y;

	//figure out direction of increment
	//incrememnt of 0 indicates either vertical or 
	//horizontal line
	if (delta_x > 0) incx = 1;
	else if (delta_x == 0) incx = 0;
	else incx = -1;

	if (delta_y > 0) incy = 1;
	else if (delta_y == 0) incy = 0;
	else incy = -1;

	//figure out which distance is greater
	delta_x = abs(delta_x);
	delta_y = abs(delta_y);

	distance = MAX(delta_x, delta_y);

	//draw line
	int curr_x = line.p1.x;
	int curr_y = line.p1.y;
	for (t = 0; t < distance + 1; t++) {
		putpixel(screen, curr_x, curr_y, color);

		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance) {
			xerr -= distance;
			curr_x += incx;
		}
		if (yerr > distance) {
			yerr -= distance;
			curr_y += incy;
		}
	}
}

void draw_triangle_int(Screen* screen, Triangle triangle, int color) {
	Line l1 = create_line(triangle.p1, triangle.p2);
	Line l2 = create_line(triangle.p2, triangle.p3);
	Line l3 = create_line(triangle.p3, triangle.p1);

	draw_line(screen, l1, color, 1);
	draw_line(screen, l2, color, 1);
	draw_line(screen, l3, color, 1);
}

void draw_triangle(Screen* screen, Triangle tri, int color, int thickness) {
	draw_triangle_int(screen, tri, color);
	return;

	//TODO fix implementation below
	
	//the max thickness of a triangle is the shortest distance
	//between the center and a vertice
	Coordinate center = triangle_center(tri);
	double l1 = line_length(create_line(center, line_center(create_line(tri.p1, tri.p2))));
	double l2 = line_length(create_line(center, line_center(create_line(tri.p2, tri.p3))));
	double l3 = line_length(create_line(center, line_center(create_line(tri.p3, tri.p1))));

	double shortest_line = MIN(l1, l2);
	shortest_line = MIN(shortest_line, l3);

	int max_thickness = shortest_line;

	//if thickness indicates shape should be filled, set to max_thickness
	if (thickness < 0) thickness = max_thickness;

	//make sure thickness isn't too big
	thickness = MIN(thickness, max_thickness);

	printf_info("max_thickness: %d", max_thickness);
	printf_info("thickness: %d", thickness);
	printf_info("center.x: %d", center.x);
	printf_info("center.y: %d", center.y);

	Coordinate p1 = tri.p1;
	Coordinate p2 = tri.p2;
	Coordinate p3 = tri.p3;	

	for (int i = 0; i < thickness; i++) {
		draw_triangle_int(screen, create_triangle(p1, p2, p3), color);

		//shrink for next shell
		p1.y += 1;
		p2.x += 1;
		p2.y -= 1;
		p3.x -= 1;
		p3.y -= 1;
	}
}

void draw_circle_int(Screen* screen, Circle circle, int color) {
	int x = 0;
	int y = circle.radius;
	int dp = 1 - circle.radius;
	do {
		if (dp < 0) {
			dp = dp + 2 * (++x) + 3;
		}
		else {
			dp = dp + 2 * (++x) - 2 * (--y) + 5;
		}

		putpixel(screen, circle.center.x + x, circle.center.y + y, color);
		putpixel(screen, circle.center.x - x, circle.center.y + y, color);
		putpixel(screen, circle.center.x + x, circle.center.y - y, color);
		putpixel(screen, circle.center.x - x, circle.center.y - y, color);
		putpixel(screen, circle.center.x + y, circle.center.y + x, color);
		putpixel(screen, circle.center.x - y, circle.center.y + x, color);
		putpixel(screen, circle.center.x + y, circle.center.y - x, color);
		putpixel(screen, circle.center.x - y, circle.center.y - x, color);
	} while (x < y);

	//put pixels at intersections of quadrants
	putpixel(screen, circle.center.x, circle.center.y - circle.radius, color);
	putpixel(screen, circle.center.x + circle.radius, circle.center.y, color);
	putpixel(screen, circle.center.x, circle.center.y + circle.radius, color);
	putpixel(screen, circle.center.x - circle.radius, circle.center.y, color);
}

void draw_circle(Screen* screen, Circle circ, int color, int thickness) {
	int max_thickness = circ.radius;
	
	//if the thickness indicates the shape should be filled, set it as such
	if (thickness < 0) thickness = max_thickness;

	//make sure they don't set one too big
	thickness = MIN(thickness, max_thickness);

	Circle c = create_circle(circ.center, circ.radius);

	for (int i = 0; i <= thickness; i++) {
		draw_circle_int(screen, c, color);

		//decrease radius for next shell
		c.radius -= 1;
	}
}

