#ifndef MINUNIT_H
#define MINUNIT_H

#define mu_assert(message, test) if (!(test)) return message;
#define mu_run_test(test) if (test()) return test();

#endif
