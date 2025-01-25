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

//  end of pc1.s
