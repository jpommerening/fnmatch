#ifndef _FNMATCH_INTERNAL_H_
#define _FNMATCH_INTERNAL_H_

fnmatch_state_t fnmatch_compile( fnmatch_pattern_t *pattern );
fnmatch_state_t fnmatch_vm_next( fnmatch_context_t *context );
fnmatch_state_t fnmatch_vm_prev( fnmatch_context_t *context );
fnmatch_state_t fnmatch_vm_retry( fnmatch_context_t *context );
fnmatch_state_t fnmatch_vm_rewind( fnmatch_context_t *context );
fnmatch_state_t fnmatch_vm_op( fnmatch_context_t *context );
fnmatch_opcode_t fnmatch_vm_opcode( fnmatch_context_t *context );

#define FNMATCH_ALLOC(mem,n,alloc) mem = _alloc( sizeof(*mem) * n, alloc )
#define FNMATCH_GROW(mem,n,alloc) mem = _grow( mem, sizeof(*mem) * n, alloc )
#define FNMATCH_CPY(dest,i,src,j,len) memcpy(&(dest[i]), &(src[j]), len*sizeof(dest[0]))

void* _alloc( size_t size, size_t* alloc );
void* _grow( void* mem, size_t size, size_t* alloc );

#endif
