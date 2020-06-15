.text
.globl exit_proc

exit_proc: 
    pushl %eax
    call exit
    ret
