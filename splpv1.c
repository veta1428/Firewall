/*
 * SPLPv1.c
 * The file is part of practical task for System programming course.
 * This file contains validation of SPLPv1 protocol.
 */

 /*
 ---------------------------------------------------------------------------------------------------------------------------
 # |      STATE      |         DESCRIPTION       |           ALLOWED MESSAGES            | NEW STATE | EXAMPLE
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 1 | INIT            | initial state             | A->B     CONNECT                      |     2     |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 2 | CONNECTING      | client is waiting for con-| A<-B     CONNECT_OK                   |     3     |
   |                 | nection approval from srv |                                       |           |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 3 | CONNECTED       | Connection is established | A->B     GET_VER                      |     4     |
   |                 |                           |        -------------------------------+-----------+----------------------
   |                 |                           |          One of the following:        |     5     |
   |                 |                           |          - GET_DATA                   |           |
   |                 |                           |          - GET_FILE                   |           |
   |                 |                           |          - GET_COMMAND                |           |
   |                 |                           |        -------------------------------+-----------+----------------------
   |                 |                           |          GET_B64                      |     6     |
   |                 |                           |        ------------------------------------------------------------------
   |                 |                           |          DISCONNECT                   |     7     |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 4 | WAITING_VER     | Client is waiting for     | A<-B     VERSION ver                  |     3     | VERSION 2
   |                 | server to provide version |          Where ver is an integer (>0) |           |
   |                 | information               |          value. Only a single space   |           |
   |                 |                           |          is allowed in the message    |           |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 5 | WAITING_DATA    | Client is waiting for a   | A<-B     CMD data CMD                 |     3     | GET_DATA a GET_DATA
   |                 | response from server      |                                       |           |
   |                 |                           |          CMD - command sent by the    |           |
   |                 |                           |           client in previous message  |           |
   |                 |                           |          data - string which contains |           |
   |                 |                           |           the following allowed cha-  |           |
   |                 |                           |           racters: small latin letter,|           |
   |                 |                           |           digits and '.'              |           |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 6 | WAITING_B64_DATA| Client is waiting for a   | A<-B     B64: data                    |     3     | B64: SGVsbG8=
   |                 | response from server.     |          where data is a base64 string|           |
   |                 |                           |          only 1 space is allowed      |           |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 7 | DISCONNECTING   | Client is waiting for     | A<-B     DISCONNECT_OK                |     1     |
   |                 | server to close the       |                                       |           |
   |                 | connection                |                                       |           |
 ---------------------------------------------------------------------------------------------------------------------------

 IN CASE OF INVALID MESSAGE THE STATE SHOULD BE RESET TO 1 (INIT)

 */


#include "splpv1.h"
#include <string.h>
#include "stdbool.h"



 /* FUNCTION:  validate_message
  *
  * PURPOSE:
  *    This function is called for each SPLPv1 message between client
  *    and server
  *
  * PARAMETERS:
  *    msg - pointer to a structure which stores information about
  *    message
  *
  * RETURN VALUE:
  *    MESSAGE_VALID if the message is correct
  *    MESSAGE_INVALID if the message is incorrect or out of protocol
  *    state
  */

enum State {
	INIT, CONNECTING, CONNECTED, WAITING_VER, WAITING_DATA, WAITING_B64_DATA, DISCONNECTING
};

static enum State state = INIT;

const char* CONNECT = "CONNECT";
const char* CONNECT_OK = "CONNECT_OK";
const char* GET_VER = "GET_VER";
const char* GET_DATA = "GET_DATA";
const char* GET_FILE = "GET_FILE";
const char* GET_COMMAND = "GET_COMMAND";
const char* GET_B64 = "GET_B64";
const char* DISCONNECT = "DISCONNECT";
const char* VERSION = "VERSION";
const char* B64 = "B64:";
const char* DISCONNECT_OK = "DISCONNECT_OK";

bool table[128] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
				   0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 
				   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
				   1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 0, 0, 0, 0, 0 };

bool tableBase64[128] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						  0, 0, 0, 1, 0, 0, 0, 1, 1, 1,
						  1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
						  0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
						  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
						  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
						  1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
						  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
						  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
						  1, 1, 1, 0, 0, 0, 0, 0 };

bool tableBase64WithEquals[128] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
									0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
									0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
									0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
									0, 0, 0, 1, 0, 0, 0, 1, 1, 1,
									1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
									0, 1, 0, 0, 0, 1, 1, 1, 1, 1,
									1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
									1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
									1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
									1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
									1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
									1, 1, 1, 0, 0, 0, 0, 0 };


enum test_status validate_message(struct Message* msg)
{
	switch (state)
	{
	case INIT:

		if (strcmp(msg->text_message, CONNECT) != 0)
		{
			return MESSAGE_INVALID;
		}
		if (msg->direction != A_TO_B)
		{
			return MESSAGE_INVALID;
		}
		state = CONNECTING;
		return MESSAGE_VALID;
	case CONNECTING:
		//A<-B CONNECT_OK 3
		//return MESSAGE_INVALID;

		if (strcmp(msg->text_message, CONNECT_OK) != 0)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		
		if (msg->direction != B_TO_A)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		state = CONNECTED;
		return MESSAGE_VALID;
		break;
	case CONNECTED:
		//A->B GET_VER 4
		//A->B GET_DATA 5
		//A->B GET_FILE 5
		//A->B GET_COMMAND 5
		//A->B GET_B64 6
		//A->B DISCONNECTS 7

		if (msg->direction != A_TO_B)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		
		if (strcmp(msg->text_message, GET_DATA) == 0 || strcmp(msg->text_message, GET_FILE) == 0 || strcmp(msg->text_message, GET_COMMAND) == 0)
		{
			state = WAITING_DATA;
			return MESSAGE_VALID;
		}
		if (strcmp(msg->text_message, DISCONNECT) == 0)
		{
			state = DISCONNECTING;
			return MESSAGE_VALID;
		}
		if (strcmp(msg->text_message, GET_B64) == 0)
		{
			state = WAITING_B64_DATA;
			return MESSAGE_VALID;
		}
		if (strcmp(msg->text_message, GET_VER) == 0)
		{
			state = WAITING_VER;
			return MESSAGE_VALID;
		}
		
		return MESSAGE_INVALID;
		
		break;
	case WAITING_VER:
		//A<-B VERSION
		//return MESSAGE_INVALID;

		if (msg->direction != B_TO_A)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}

		if (strncmp(msg->text_message, VERSION, 7) == 0)
		{
			if (msg->text_message[7] != ' ') {
				state = INIT;
				return MESSAGE_INVALID;
			}
			for (size_t i = 8; msg->text_message[i] != '\0'; i++)
			{
				if (!(msg->text_message[i] >= 48 && msg->text_message[i] < 58) || msg->text_message[i] == 32)
				{
					state = INIT;
					return MESSAGE_INVALID;
				}
			}

			state = CONNECTED;
			return MESSAGE_VALID;
		}
		else {
			state = INIT;
			return MESSAGE_INVALID;
		}
		break;
	case WAITING_DATA:
		//A<-B CMD data CMD

		if (msg->direction != B_TO_A)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		if (strncmp(msg->text_message, GET_DATA, 8) == 0)
		{
			if (msg->text_message[8] != ' ') {
				state = INIT;
				return MESSAGE_INVALID;
			}
			char* pointer = msg->text_message + 9;
			for (; *pointer != ' ' && *pointer != '\0';)
			{
				if (!*(table + *pointer))
				{
					state = INIT;
					return MESSAGE_INVALID;
				}
				pointer++;
			}

			if (*pointer == '\0' || strcmp(pointer + 1, GET_DATA)) {
				state = INIT;
				return MESSAGE_INVALID;
			}
			state = CONNECTED;
			return MESSAGE_VALID;
		}
		if (strncmp(msg->text_message, GET_FILE, 8) == 0)
		{
			if (msg->text_message[8] != ' ') {
				state = INIT;
				return MESSAGE_INVALID;
			}
			char* pointer = msg->text_message + 9;
			for (; *pointer != ' ' && *pointer != '\0';)
			{
				if (!*(table + *pointer))
				{
					state = INIT;
					return MESSAGE_INVALID;
				}
				pointer++;
			}

			if (*pointer == '\0' || strcmp(pointer + 1, GET_FILE)) {
				state = INIT;
				return MESSAGE_INVALID;
			}
			state = CONNECTED;
			return MESSAGE_VALID;

		}
		if (strncmp(msg->text_message, GET_COMMAND, 11) == 0)
		{
			if (msg->text_message[11] != ' ') {
				state = INIT;
				return MESSAGE_INVALID;
			}

			char* pointer = msg->text_message + 12;
			for (; *pointer != ' ' && *pointer != '\0';)
			{
				if (!*(table + *pointer))
				{
					state = INIT;
					return MESSAGE_INVALID;
				}
				pointer++;
			}

			if (*pointer == '\0' || strcmp(pointer + 1, GET_COMMAND)) {
				state = INIT;
				return MESSAGE_INVALID;
			}
			state = CONNECTED;
			return MESSAGE_VALID;
		}

		break;
	case WAITING_B64_DATA:
		//A<-B B64 

		if (msg->direction != B_TO_A)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		if (strncmp(msg->text_message, B64, 4) != 0)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		if (msg->text_message[4] != ' ') {
			state = INIT;
			return MESSAGE_INVALID;
		}
		char* initialPointer = msg->text_message + 5;
		char* pointer = msg->text_message + 5;
		for (; *(pointer + 2) != '\0';)
		{
			if (!*(tableBase64 + *pointer))
			{
				state = INIT;
				return MESSAGE_INVALID;
			}
			pointer++;
		}

		if (!*(tableBase64 + *pointer))
		{
			if (!(*pointer == '=' && *(pointer + 1) == 61))
			{
				state = INIT;
				return MESSAGE_INVALID;
			}
		}
		else if (!*(tableBase64WithEquals + *(pointer + 1)))
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		if ((pointer + 2 - initialPointer) % 4 != 0)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		state = CONNECTED;
		return MESSAGE_VALID;
		break;
	case DISCONNECTING:
		//A<-B DISCONNECT_OK 1
		//A<-B CONNECT_OK 3

		if (strcmp(msg->text_message, DISCONNECT_OK) != 0)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		if (msg->direction != B_TO_A)
		{
			state = INIT;
			return MESSAGE_INVALID;
		}
		state = INIT;
		return MESSAGE_VALID;
		break;
	default:
		break;
	}

	return MESSAGE_VALID;
}
