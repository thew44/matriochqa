#ifndef CSVPARSER_H
#define CSVPARSER_H

#include "3rdparty/fast-cpp-csv-parser/csv.h"
typedef io::CSVReader<14, io::trim_chars<' '>, io::no_quote_escape<';'>> csvreader_t;

#endif // CSVPARSER_H
