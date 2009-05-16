// ------------------------------------------------------------------------- //
#ifndef VERSION_H
#define VERSION_H

#define GUAYADEQUE_STATUS " Beta"
#define SVN_VERSION

#ifndef SVN_VERSION
#define GUAYADEQUE_VERSION "v0.0.5($DATE)" GUAYADEQUE_STATUS
#else
#define GUAYADEQUE_VERSION "svn($DATE)" GUAYADEQUE_STATUS
#endif

#endif
// ------------------------------------------------------------------------- //
