#if !defined(CONIOTERMIOS_H)
#define CONIOTERMIOS_H

#if !defined _WIN32 

void set_conio_termial_mode(void);
void reset_terminal_mode(void);
int kbhit(void);
int getch(void);

#endif

#endif

