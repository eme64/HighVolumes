#include "cacheUpdateCoord.h"

void PolytopeJIT_generate_cacheUpdateCoord_ref(const Polytope *p, PolytopeJIT *o) {
   //jit_print();
   
   o->cacheUpdateCoord = (pjit_cacheUpdateCoord_f_t)jit_head();
   
   // info about calling convention:
   // d -> %rdi
   // dx -> %xmm0
   // cache -> %rsi

   // required computation:
   // jump according to d.
   // update relevant cache entries
   // return
   
   ///  // ---------------- test 
   ///  // f2 0f 11 06          	movsd  %xmm0,(%rsi)
   ///  { uint8_t instr[] = {0xf2,0x0f,0x11,0x06}; jit_push(instr,4); }
   ///  // ---- rep ret
   ///  { uint8_t instr[] = {0xf3,0xc3}; jit_push(instr,2); }
   ///  return;

   // ------------------------------------------ switch case head
   assert(p->n < 256 && "if this asserts, then extend for n larger!");
   uint8_t nn = p->n;
   // 0x83,0xff,xx   cmp    xx,%edi
   {const uint8_t instr[] = {0x83,0xff,nn}; jit_push(instr,3);}
   
   // -------------- if bad, jump to end
   // 0f 87 xx xx xx xx    	ja  xxxx   -- relative jump
   {const uint8_t instr[] = {0x0f,0x87,0,0,0,0}; jit_push(instr,6);}
   uint8_t* jump_end = jit_head();// prepare to set L_end here
   
   // ------ get ref to jump table
   // 48 8d 05 xx xx xx xx 	lea    xxxx(%rip),%rax
   {const uint8_t instr[] = {0x48,0x8d,0x05,0,0,0,0}; jit_push(instr,7);}
   uint8_t* set_table = jit_head();// prepare to set L_table here
   // 89 ff                	mov    %edi,%edi
   {const uint8_t instr[] = {0x89,0xff}; jit_push(instr,2);}
   // 4c 63 1c b8          	movslq (%rax,%rdi,4),%r11
   {const uint8_t instr[] = {0x4c,0x63,0x1c,0xb8}; jit_push(instr,4);}
   // 49 01 c3             	add    %rax,%r11
   {const uint8_t instr[] = {0x49,0x01,0xc3}; jit_push(instr,3);}
   // 41 ff e3             	jmpq   *%r11
   {const uint8_t instr[] = {0x41,0xff,0xe3}; jit_push(instr,3);}


   // -------------------------------------------------- Jump table
   jit_allign(4);// allign for array of longs below
   uint8_t* table = jit_head();
   uint32_t L_table = jit_head() - set_table;
   jit_write(set_table-4, (uint8_t*)&L_table,4);
   for(int i=0;i<p->n;i++) {
      {const uint8_t instr[] = {1,1,1,1}; jit_push(instr,4);}
   }
   
   // ------------------------------ dump code for each i
   uint8_t* jump[p->n];
   for(int i=0;i<p->n;i++) {// generate code for each, register location in table
      uint8_t* location = jit_head();
      uint32_t entry = location - table;
      jit_write(table+4*i, (uint8_t*)&entry, 4);
      
      // find relevant entries in column:
      for(int j=0;j<p->m;j++) {
         FT aij = Polytope_get_a(p,j,i);
	 if(aij != 0.0) {
	 
	    //movabs $0xff00ff00ff00ff00,%rax
            {const uint8_t instr[] = {0x48,0xb8}; jit_push(instr,2); }
	    jit_push((const uint8_t*)&aij,8);
            // c4 e1 f9 6e e0       	vmovq  %rax,%xmm4
            {const uint8_t instr[] = {0xc4,0xe1,0xf9,0x6e,0xe0}; jit_push(instr,5);}
            
	    // goal:
	    // fmadd: cachej += xmm4 * xmm0
	    // 
	    // asm:  xmm4 = cachej + xmm0*xmm4
	    // c4 e2 f9 a9 a6 xx xx xx xx  vfmadd213sd 0x100(%rsi),%xmm0,%xmm4
            {const uint8_t instr[] = {0xc4,0xe2,0xf9,0xa9,0xa6}; jit_push(instr,5); }
	    uint32_t cachej = 8*j;
	    jit_push((const uint8_t*)&cachej,4);
	    // f2 0f 11 a6 xx xx xx xx     movsd  %xmm4,0x100(%rsi)
            {const uint8_t instr[] = {0xf2,0x0f,0x11,0xa6}; jit_push(instr,4); }
	    jit_push((const uint8_t*)&cachej,4);
	 }
      }
      
      // ---- rep ret
      { uint8_t instr[] = {0xf3,0xc3}; jit_push(instr,2); }
   }
   
   // ---------------------------------------- set L_end:
   uint32_t offset = jit_head() - jump_end;
   uint64_t offset64 = jit_head() - jump_end;
   //printf("jump offset: %d %ld\n",offset, offset64);
   jit_write(jump_end-4, (uint8_t*)&offset, 4);
   
   // ---- rep ret
   { uint8_t instr[] = {0xf3,0xc3}; jit_push(instr,2); }
   
   //jit_print();
}

