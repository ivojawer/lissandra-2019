#ifndef REQUESTS_H_
#define REQUESTS_H_

#define SELECT 0
#define INSERT 1
#define CREATE 2
#define DESCRIBE 3
#define DROP 4
#define JOURNAL 5
#define ADD 6

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>


int esUnaRequestYCual(char* palabra);


#include "requests.h"

#endif /* REQUESTS_H_ */
