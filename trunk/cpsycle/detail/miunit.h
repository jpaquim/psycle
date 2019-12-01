///\file: miunit.h
///\brief MinUnit -- a minimal unit testing framework for C by JeraDesign

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); test_run++; \
								if (message) return message; } while (0)

extern int test_run;


