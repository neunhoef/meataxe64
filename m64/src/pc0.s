/* pc1.s x86 assembler non-HPMI functions*/
// parms rdi rsi rdx rcx r8 r9 
// scratch rax (RV) r10 r11
// not scratch rbx rbp r12 r13 r14 r15

// mactype (char res[8])
//             rdi 

	.text
	.globl	mactype
mactype:
        pushq   %rbx
        movb    $0x61,%r8b    /* Class starts off as 'a'  */

/*  check lahf-lm, cx16 and sse3 to move to class 'b'     */
        movl    $0x80000001,%eax
        cpuid

        testl   $1,%ecx       /* lahf-lm set? */
        je      macp2         /* if not, class is 'a' */
        movl    $1,%eax
        cpuid
        movl    %ecx,%r9d     /* save ecx for later in r9 */
        movl    %eax,%r10d    /* family stuff for later */
        testl   $0x2000,%ecx  /* cx16 bit set?  */
        je      macp2         /* if not, class is 'a' */
        testl   $1,%ecx       /* SSE3 - pni bit set?  */
        je      macp2         /* if not, class is 'a' */
        movb    $0x62,%r8b    /* Class at least 'b'  */

/*  check SSSE3 to move to class 'c'  */
        testl   $0x200,%ecx   /* SSSE3 - bit set?  */
        je      macp2         /* if not, class is 'b' */
        movb    $0x63,%r8b    /* Class at least 'c'  */
/*  check SSE4.1 to move to class 'd'  */
        testl   $0x80000,%ecx   /* SSE4.1 - bit set?  */
        je      macp2         /* if not, class is 'c' */
        movb    $0x64,%r8b    /* Class at least 'd'  */
/*  check SSE4.2 to move to class 'e'  */
        testl   $0x100000,%ecx   /* SSE4.2 - bit set?  */
        je      macp2         /* if not, class is 'd' */
        movb    $0x65,%r8b    /* Class at least 'e'  */
/*  check CLMUL to move to class 'f'  */
        testl   $0x2,%ecx     /* CLMUL - bit set?  */
        je      macp2         /* if not, class is 'e' */
        movb    $0x66,%r8b    /* Class at least 'f'  */
/*  check AVX-1 to move to class 'g'  */
        testl   $0x10000000,%ecx     /* AVX - bit set?  */
        je      macp2         /* if not, class is 'e' */
        movb    $0x67,%r8b    /* Class at least 'g'  */
/*  check BMI1 to move to class 'h'  */
        movl    $0x7,%eax
        movl    $0,%ecx
        cpuid
        movl    %r10d,4(%rdi)
        testl   $8,%ebx       /* check bmi1 bit */
        je      macp2         /* not set - class is 'g' */
        movb    $0x68,%r8b    /* else class is at least 'h' */
/*  check BMI2 to move to class 'i'  */
        testl   $0x100,%ebx     /* BMI2 - bit set?  */
        je      macp1         /* if not, class is 'h' or 'k' */
        movb    $0x69,%r8b    /* Class at least 'i'  */
/*  check AVX2 and MOVBE to move to class 'j'  */
        testl   $0x20,%ebx     /* AVX2 - bit set?  */
        je      macp1         /* if not, check FMA */
        testl   $0x400000,%r9d /* test MOVBE bit */
        je      macp1         /* class j needs MOVBE as well */
        movb    $0x6A,%r8b    /* Class at least 'j'  */
        testl   $0x1000,%r9d /* test FMA bit */
        je      macp2        /* no fma, class 'j' */
        movb    $0x6C,%r8b   /* class 'l' if all of them */
        testl   $0x10000,%ebx  /* AVX512 available? */
        je      macp2        /* if not, class 'l' */
        movb    $0x6D,%r8b   /* class 'm' if it is there */  
/* future tests in advance of 'm' go here */
        jmp     macp2   
macp1:                       /* no BMI2/AVX2/MOVBE but FMA? */
        testl   $0x1000,%r9d /* test FMA bit */
        je      macp2        /* no fma, class 'i' */
        movb    $0x6B,%r8b   /* class 'k' if fma but not avx2/movbe */

macp2:
        movb    %r8b,(%rdi)   /* put class into mact[0]   */
        movl    %r10d,%eax
        shrl    $20,%eax
        shrl    $8,%r10d
        andl    $15,%r10d
        andl    $255,%eax
        addl    %r10d,%eax    /* so eax is the family number */
        movb    $0x32,1(%rdi) /* put cache indicator = '2' */
        cmpl    $0x15,%eax    /* for Bulldozer-Steamroller family */
        je      macp4
        movb    $0x31,1(%rdi) /* put cache indicator = '1' */
        cmpl    $0x17,%eax    /* for Ryzen family */
        je      macp4
        movb    $0x30,1(%rdi) /* else cache indicator = '0' */
macp4:
        popq    %rbx
        ret

// pc5bmdd Afmt bwa Cfmt parms
// SSE pmulld slower but can do 10-bit

/* %rdi -> Afmt     %rsi bwa     %rdx -> Cfmt  %rcx parms */
/* %rax Afmt        %r8 slice in bwa  %r11 constant bwa stride */
/* %rbx counter for slices  */ 
/* %r9 used for skip terminate then %r9,%r10 add/subtract displacement */

// untouchted rbp r12 r13 r14 r15
	.text
	.globl	pc5bmdd
pc5bmdd:
        pushq         %rbx
        movq        16(%rcx),%xmm8    /* mask     */
        pshufd      $0x44,%xmm8,%xmm8
        movq       8(%rcx),%xmm9      /* shift S  */
        movq        56(%rcx),%xmm10     /* 2^S % p  */
        pshufd      $0x44,%xmm10,%xmm10
        movq        64(%rcx),%xmm11     /* bias     */
        pshufd      $0x44,%xmm11,%xmm11 
        movq         32(%rcx),%r11     /* size of one slot */
        imul         40(%rcx),%r11     /* times slots = slice stride */

        movq    0(%rdi),%rax  /* first word of Afmt   */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        je      pc5bmdd8      /* yes get straight out */
//  Start of secondary loop
pc5bmdd1:
        shlq    $7,%r9        /* multiply by 128      */
        addq    %r9,%rdx      /* add into Cfmt addr   */
        movq    %rsi,%r8      /* copy BWA addr to %r8 */
        addq    $8,%rdi       /* point to next alcove */
        movq    $7,%rbx        /* number of slices  */

        movdqa   0(%rdx),%xmm0   /* get cauldron of Cfmt */
        movdqa  16(%rdx),%xmm1
        movdqa  32(%rdx),%xmm2
        movdqa  48(%rdx),%xmm3
        movdqa  64(%rdx),%xmm4
        movdqa  80(%rdx),%xmm5
        movdqa  96(%rdx),%xmm6
        movdqa 112(%rdx),%xmm7

//  Start of primary loop
pc5bmdd2:
        movq    %rax,%r9      /* copy Afmt to add index */
        shrq    $4,%r9        /* top nybble to bottom   */
        andq    $0x780,%r9     /* the nybble            */
        movq    %rax,%r10     /* copy Afmt to sub index */
        andq    $0x780,%r10     /* get bottom nybble      */
        shrq    $8,%rax       /* next byte of Afmt      */ 
        paddq    0(%r8,%r9),%xmm0    /* add in cauldron */
        paddq   16(%r8,%r9),%xmm1
        paddq   32(%r8,%r9),%xmm2
        paddq   48(%r8,%r9),%xmm3
        paddq   64(%r8,%r9),%xmm4
        paddq   80(%r8,%r9),%xmm5
        paddq   96(%r8,%r9),%xmm6
        paddq  112(%r8,%r9),%xmm7
        psubq    0(%r8,%r10),%xmm0
        psubq   16(%r8,%r10),%xmm1
        psubq   32(%r8,%r10),%xmm2
        psubq   48(%r8,%r10),%xmm3
        psubq   64(%r8,%r10),%xmm4
        psubq   80(%r8,%r10),%xmm5
        psubq   96(%r8,%r10),%xmm6
        psubq  112(%r8,%r10),%xmm7
        addq    %r11,%r8                 /* move on to next slice */
        subq    $1,%rbx
        jne     pc5bmdd2
//  End of primary loop

        movdqa  %xmm0,%xmm12
        movdqa  %xmm1,%xmm13
        movdqa  %xmm2,%xmm14
        movdqa  %xmm3,%xmm15
        pand    %xmm8,%xmm12
        pand    %xmm8,%xmm13
        pand    %xmm8,%xmm14
        pand    %xmm8,%xmm15
        pxor    %xmm12,%xmm0
        pxor    %xmm13,%xmm1
        pxor    %xmm14,%xmm2
        pxor    %xmm15,%xmm3
        psrld   %xmm9,%xmm12
        psrld   %xmm9,%xmm13
        psrld   %xmm9,%xmm14
        psrld   %xmm9,%xmm15
        pmulld  %xmm10,%xmm12
        pmulld  %xmm10,%xmm13
        pmulld  %xmm10,%xmm14
        pmulld  %xmm10,%xmm15
        paddq   %xmm12,%xmm0
        paddq   %xmm13,%xmm1
        paddq   %xmm14,%xmm2
        paddq   %xmm15,%xmm3
        paddq   %xmm11,%xmm0
        paddq   %xmm11,%xmm1
        paddq   %xmm11,%xmm2
        paddq   %xmm11,%xmm3
        movdqa  %xmm0,0(%rdx)
        movdqa  %xmm1,16(%rdx)
        movdqa  %xmm2,32(%rdx)
        movdqa  %xmm3,48(%rdx)

        movdqa  %xmm4,%xmm12
        movdqa  %xmm5,%xmm13
        movdqa  %xmm6,%xmm14
        movdqa  %xmm7,%xmm15
        pand    %xmm8,%xmm12
        pand    %xmm8,%xmm13
        pand    %xmm8,%xmm14
        pand    %xmm8,%xmm15
        pxor    %xmm12,%xmm4
        pxor    %xmm13,%xmm5
        pxor    %xmm14,%xmm6
        pxor    %xmm15,%xmm7
        psrld   %xmm9,%xmm12
        psrld   %xmm9,%xmm13
        psrld   %xmm9,%xmm14
        psrld   %xmm9,%xmm15
        pmulld  %xmm10,%xmm12
        pmulld  %xmm10,%xmm13
        pmulld  %xmm10,%xmm14
        pmulld  %xmm10,%xmm15
        paddq   %xmm12,%xmm4
        paddq   %xmm13,%xmm5
        paddq   %xmm14,%xmm6
        paddq   %xmm15,%xmm7
        paddq   %xmm11,%xmm4
        paddq   %xmm11,%xmm5
        paddq   %xmm11,%xmm6
        paddq   %xmm11,%xmm7
        movdqa  %xmm4,64(%rdx)
        movdqa  %xmm5,80(%rdx)
        movdqa  %xmm6,96(%rdx)
        movdqa  %xmm7,112(%rdx)

        movq    0(%rdi),%rax  /* next word of Afmt    */
        movzbq  %al,%r9       /* get skip/terminate   */
        shrq    $1,%rax
        cmpq    $255,%r9      /* is it terminate?     */
        jne     pc5bmdd1         /* no - round again     */
//  End of secondary loop
pc5bmdd8:
        popq    %rbx
        ret      

// end of pc5bmdd
