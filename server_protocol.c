#include "server_protocol.h"
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include "board_representation_maker.h"

#define PUT_ROW_BUFFER_IDX 0
#define PUT_COL_BUFFER_IDX 1
#define PUT_NUMB_BUFFER_IDX 2

//forward declarations

uint32_t calculate_str_len(char* str);

int process_message(server_protocol_t* protocol, char message);

int process_p_message(server_protocol_t* protocol);
int process_g_message(server_protocol_t* protocol);
int process_r_message(server_protocol_t* protocol);
int process_v_message(server_protocol_t* protocol);

int show_board_to_client(server_protocol_t* protocol);

int send_message_to_client(server_protocol_t* protocol, char* mes);
int send_unmodifiable_cell_message(server_protocol_t* protocol);
int send_modifiable_cell_message(server_protocol_t* protocol);
int send_invalid_board_message(server_protocol_t* protocol);
int send_valid_board_message(server_protocol_t* protocol);

int server_protocol_init(server_protocol_t* protocol, const char* port, int matrix[9][9]) {
	sudoku_init(&protocol->sudoku, matrix);
	if (server_init(&protocol->sv, port)) {
		return 1;
	}
	server_accept_client(&protocol->sv);
	return 0;
}

void server_protocol_release(server_protocol_t* protocol) {
	server_release(&protocol->sv);
}

int server_protocol_process(server_protocol_t* protocol) {
	char buffer;
	if (server_recv_message(&protocol->sv, &buffer, 1)) {
		server_release(&protocol->sv);
		return 1;
	}
	if (process_message(protocol, buffer)) {
		server_release(&protocol->sv);
		return 1;
	}
	return 0;
}

int process_message(server_protocol_t* protocol, char message) {
	if (message == 'P') {
		return process_p_message(protocol);
	}
	else if (message == 'G') {
		return process_g_message(protocol);
	}
	else if (message == 'R') {
		return process_r_message(protocol);
	}
	else {
		return process_v_message(protocol);
	}
}


int process_p_message(server_protocol_t* protocol) {
	char buffer[3];
	if (server_recv_message(&protocol->sv, buffer, 3)) {
		return 1;
	}
	int numb = buffer[PUT_NUMB_BUFFER_IDX] - '0'; //here, we parse the value stored as an ascii code
	int row = buffer[PUT_ROW_BUFFER_IDX] - '0';
	int col = buffer[PUT_COL_BUFFER_IDX] - '0';

	int unmodifiable_cell = sudoku_put(&protocol->sudoku, numb, row, col);
	if (unmodifiable_cell) {
		return send_unmodifiable_cell_message(protocol);
	} else {
		return send_modifiable_cell_message(protocol);
	}
}

int send_unmodifiable_cell_message(server_protocol_t* protocol) {
	char mes[] = "La celda indicada no es modificable\n​";
	if (send_message_to_client(protocol, mes) == 1) {
		return 1;
	}
	return 0;
}

int send_modifiable_cell_message(server_protocol_t* protocol) {
	return show_board_to_client(protocol);
}

int process_g_message(server_protocol_t* protocol) {
	return show_board_to_client(protocol);
}

int process_r_message(server_protocol_t* protocol) {
	sudoku_restart(&protocol->sudoku);
	return show_board_to_client(protocol);
}

int process_v_message(server_protocol_t* protocol) {
	if (sudoku_verify(&protocol->sudoku)) {
		return send_invalid_board_message(protocol);
	}
	return send_valid_board_message(protocol);
}

int send_invalid_board_message(server_protocol_t* protocol) {
	char* mes = "​ERROR\n​";
	if (send_message_to_client(protocol, mes) == 1) {
		return 1;
	}
	return 0;
}

int send_valid_board_message(server_protocol_t* protocol) {
	char* mes = "OK\n";
	if (send_message_to_client(protocol, mes) == 1) {
		return 1;
	}
	return 0;
}


int send_message_to_client(server_protocol_t* protocol, char* mes) {
	uint32_t len = calculate_str_len(mes);
	uint32_t len_ton = htonl(len);
	if (server_send_message(&protocol->sv, (char*)&len_ton, 4)) {
		return 1;
	}
	for (int i = 0; i < len; i++) {
		if (server_send_message(&protocol->sv, &mes[i], 1)) {
			return 1;
		}
	}
	return 0;
}

int show_board_to_client(server_protocol_t* protocol) {
	int matrix[9][9];
	sudoku_show_board(&protocol->sudoku, matrix);
	char* board_representation = assemble_board_representation(matrix);
	int err = send_message_to_client(protocol, board_representation);
	free(board_representation);
	return (err == 1);
}
	
uint32_t calculate_str_len(char* str) {
	uint32_t len = 0;
	int i = 0;
	while (str[i] != '\0') {
		len += 1;
		i++;
	}
	return len;
}