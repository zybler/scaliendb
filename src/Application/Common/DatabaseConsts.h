#ifndef DATABASECONSTS_H
#define DATABASECONSTS_H

#include "System/Common.h"

#define DATABASE_NAME_SIZE          ( 1*KiB)

#define DATABASE_KEY_SIZE           ( 1*KiB)
#define DATABASE_VAL_SIZE           (16*MiB)

#define DATABASE_REPLICATION_SIZE   (1*KiB + DATABASE_KEY_SIZE + DATABASE_VAL_SIZE)

#endif