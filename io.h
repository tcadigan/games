#ifndef IO_H_
#define IO_H_

void clear_screen(int row, int col);
void pause_exit(int prt_line, int delay);
void inkey(char *ch);
void move_cursor(int row, int col);
void put_buffer(char *out_str, int row, int col);
void prt(char *str_buf, int row, int col);
void msg_print(char *str_buf);
void flush();
void print(char *str_buf, int row, int col);

#endif
