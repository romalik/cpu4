/* hostfile for cpu4 */

#include <string.h>

static char rcsid[] = "";

#ifndef LCCDIR
#define LCCDIR "/opt/cpu4/bin/"
#endif

#ifndef LCCLIBDIR
#define LCCLIBDIR "/opt/cpu4/lib/"
#endif

char *suffixes[] = { ".c", ".i", ".s", ".o", ".out", 0 };
char inputs[256] = "";
char *cpp[] = { LCCDIR "cpp",
   "-D__STDC__=1",
   "$1", "$2", "$3", 0 };
char *include[] = { "-I" LCCDIR "include", 0 };
char *com[] = { LCCDIR "rcc", "-target=cpu4",
   "$1", "$2", "$3", 0 };
char *as[] = { LCCDIR "cpu4as", "$1", "$2", "$3", 0 };

char obj_begin[] = LCCLIBDIR"crt0.o";

char obj_end[] = LCCLIBDIR"crt_end.o";

char *ld[] = { LCCDIR "cpu4ld", "-o", "$3", obj_begin, LCCLIBDIR "crt1.o", "$1", "$2", /*obj_end,*/ 0};




extern char *concat(char *, char *);

int option(char *arg) {
   if (strncmp(arg, "-lccdir=", 8) == 0) {
       cpp[0] = concat(&arg[8], "/cpp");
       include[0] = concat("-I", concat(&arg[8], "/include"));
       com[0] = concat(&arg[8], "/rcc");
   }
   else if (strncmp(arg, "-ld=", 4) == 0)
       ld[0] = &arg[4];
   else if (strncmp(arg, "-kernel", 7) == 0) {
       //ld[3] = kernbeg;
       //ld[8] = kernend;
   } else
       return 0;
   return 1;
}

