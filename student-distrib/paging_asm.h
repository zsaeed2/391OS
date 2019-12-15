#ifndef _PAGING_ASM_H
#define _PAGING_ASM_H

#ifndef ASM

//x86 code to enable paging
extern void enablePaging(void);
//load page directory
extern void loadPageDirectory(unsigned int*);

#endif
#endif
