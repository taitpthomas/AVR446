#ifndef OPTIONS_H
#define OPTIONS_H

struct motor_options{
	float turn;
	float accel;
	float decel;
	float speed;
};

int get_motor_options(int argc, char **argv, struct motor_options *p);

#endif
