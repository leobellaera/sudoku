#include "cell.h"

void cell_init(cell_t* cell, int number, bool modifiable) {
	cell->modifiable = modifiable;
	cell->number = number;
}

int cell_set_number(cell_t* cell, int numb) {
	if (!cell->modifiable) {
		return 1;
	}
	cell->number = numb;
	return 0;
}

int cell_get_number(cell_t* cell) {
	return cell->number;
}

bool cell_is_valid(cell_t* cell_a, cell_t* cell_b) {
	return (cell_a->number != cell_b->number || cell_a->number == 0);
}


void cell_restart(cell_t* cell) {
	if (cell->modifiable) {
		cell->number = 0;
	}
}
