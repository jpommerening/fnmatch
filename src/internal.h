#ifndef _FNMATCH_INTERNAL_H_
#define _FNMATCH_INTERNAL_H_

#define FNMATCH_ESCAPE '\\'
#define FNMATCH_CHARS_START '['
#define FNMATCH_CHARS_END ']'
#define FNMATCH_CHARS_NEGATE '!'
#define FNMATCH_ONE '?'
#define FNMATCH_ANY '*'
#define FNMATCH_DEEP '*'
#define FNMATCH_SEP '/'

void* fnmatch_compile( const char* expr, int flags, size_t* length, fnmatch_stats_t* stats );

fnmatch_state_t fnmatch_vm_next( fnmatch_context_t *context );
fnmatch_state_t fnmatch_vm_prev( fnmatch_context_t *context );
fnmatch_state_t fnmatch_vm_retry( fnmatch_context_t *context );
fnmatch_state_t fnmatch_vm_rewind( fnmatch_context_t *context );
fnmatch_state_t fnmatch_vm_op( fnmatch_context_t *context );

#endif
