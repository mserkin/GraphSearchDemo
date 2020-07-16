/*
 * error.h
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#ifndef INC_ERROR_H_
#define INC_ERROR_H_

constexpr int NO_ERROR = 0;
constexpr int FATAL_ERROR_FILE_OPEN_FAILURE = -1;
constexpr int FATAL_ERROR_NO_MEMORY = -2;
constexpr int FATAL_ERROR_FILE_READ_FAILURE = -3;
constexpr int FATAL_ERROR_ROOT_IS_NOT_ARRAY = -4;
constexpr int FATAL_ERROR_NAME_ELEMENT_NOT_FOUND = -5;
constexpr int FATAL_ERROR_FROM_VERTEX_NOT_FOUND = -6;
constexpr int FATAL_ERROR_TO_VERTEX_NOT_FOUND = -7;
constexpr int FATAL_ERROR_FAILED_TO_ADD_VERTEX = -8;
constexpr int FATAL_ERROR_FAILED_TO_ADD_EDGE = -9;
constexpr int FATAL_ERROR_UNKNOWN_FILE_TYPE = -10;
constexpr int FATAL_ERROR_SELF_LOOPS_ARE_PROHIBITED = -101;
constexpr int FATAL_ERROR_BIDIRECTION_EDGES_ARE_PROHIBITED = -102;
constexpr int FATAL_ERROR_EDGE_ALREADY_EXISTS = -103;
constexpr int FATAL_ERROR_2D_FILE_CONTAINS_SHORT_LINE = -201;
constexpr int FATAL_ERROR_2D_FILE_IS_SHORT = -201;

#endif /* INC_ERROR_H_ */
