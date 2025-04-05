#ifndef CNT_FILE_READER_H
#define CNT_FILE_READER_H

void read_cnt_file(const char* filename, void (*process_line)(const char*, const char*));

#endif