# GDB initialization script with commands to make debugging GCC4MBED applications more convenient.
#
# This gdbinit file is placed here in the sample folder of the repository so that it can be included from the various
# locations in the source tree from which a user might want to launch make and GDB. The "source" command in GDB
# can be used to manually load this script when needed.
#
# Commands of Interest
#   threads - Displays information about all active RTX/mbed-os threads.
#   thread [threadPointer] - Switches current thread context.
#   heapsize - Displays the current heap size.
#   heapwalk - Walks the heap and dumps each chunk encountered.
#   showfault - Dumps descriptive text about the cause of a hard fault.
#   catchfaults - Sets Cortex-M device (doesn't work for Cortex-M0/M0+ devices) to halt in debugger on faults.
#
# Issuing "help user-defined" command from within GDB will list the commands added by this script.
#
# Initially created by Adam Green - June 2017.



# Command to list active RTX/mbed-os threads.
define threads
    # Remember the current thread so that we can switch back to it after showing stacks for all threads.
    if ($thread_swapped == 1)
        # We have already swapped context to a new thread. Make sure to switch back to it at the end of this function.
        set var $thread_orig_context=$thread_ptr
    else
        # We are currently using original context from the debugger stop so switch back to it at end of this function.
        set var $thread_orig_context=0
    end
    
    # Start by looking at threads in running state.
    # Usually curr and next will be the same but they can differ if RTX is in the process of switching the context.
    # When in the middle of a context switch, these threads may show up on other lists as well. The code below will
    # ignore these threads if they do show up again on another list so that they aren't displayed multiple times.
    set var $thread_run_curr=osRtxInfo.thread.run.curr
    set var $thread_run_next=osRtxInfo.thread.run.next
    thread_display $thread_run_curr
    if ($thread_run_curr != $thread_run_next)
        thread_display $thread_run_next
    end

    # Walk the ready list of threads.
    set var $thread_curr=osRtxInfo.thread.ready.thread_list
    while ($thread_curr != 0)
        if ($thread_curr != $thread_run_curr && $thread_curr != $thread_run_next)
            thread_display $thread_curr
        end
        set var $thread_curr=$thread_curr->thread_next
    end

    # Walk the delay list of threads. These are the threads waiting on objects with a timeout.
    set var $thread_curr=osRtxInfo.thread.delay_list
    while ($thread_curr != 0)
        if ($thread_curr != $thread_run_curr && $thread_curr != $thread_run_next)
            thread_display $thread_curr
        end
        set var $thread_curr=$thread_curr->delay_next
    end
    
    # Walk the waiting list of threads. These are the threads waiting on objects with an infinite timeout.
    set var $thread_curr=osRtxInfo.thread.wait_list
    while ($thread_curr != 0)
        if ($thread_curr != $thread_run_curr && $thread_curr != $thread_run_next)
            thread_display $thread_curr
        end
        set var $thread_curr=$thread_curr->delay_next
    end

    # Walk the list of terminating threads.
    set var $thread_curr=osRtxInfo.thread.terminate_list
    while ($thread_curr != 0)
        if ($thread_curr != $thread_run_curr && $thread_curr != $thread_run_next)
            thread_display $thread_curr
        end
        set var $thread_curr=$thread_curr->thread_next
    end

    # Switch the context back to where it was when this function started.
    if ($thread_orig_context != 0)
        # The extra argument of 0 tells thread command to just switch thread context but skip dumping 
        # the callstack again.
        thread $thread_orig_context 0
    else
        thread_restore
    end
end

# Display key items of interest about the specified RTX thread.
# Meant to be used internally by the above "threads" command and not directly by user.
define thread_display
    set var $thread_tcb=(osRtxThread_t*)$arg0

    printf "        THREAD: 0x%08X", $thread_tcb
    if ($thread_tcb->name != 0)
        printf "    \"%s\"\n", $thread_tcb->name
    else
        printf "\n"
    end
    printf "      priority: %2u          priority base: %u\n", $thread_tcb->priority, $thread_tcb->priority_base
    printf "     stack top: 0x%08X     stack size: %u\n", $thread_tcb->stack_mem + $thread_tcb->stack_size, $thread_tcb->stack_size
    printf " stack current: 0x%08X     stack free: %d\n", $thread_tcb->sp, $thread_tcb->sp - (unsigned int)$thread_tcb->stack_mem
    printf "    exc_return: 0x%02X", $thread_tcb->stack_frame
    if (($thread_tcb->stack_frame & 0x10) == 0)
        printf " (FPU stacked)\n"
    else
        printf "\n"
    end
    printf "         state: "

    # Some states are part of the osThreadState_t enumeration and others are specific to RTX.
    if ($thread_tcb->state <= osThreadTerminated)
        output (osThreadState_t)$thread_tcb->state
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x10U))
        printf "osRtxThreadWaitingDelay"
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x20U))
        printf "osRtxThreadWaitingJoin"
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x30U))
        printf "osRtxThreadWaitingThreadFlags"
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x40U))
        printf "osRtxThreadWaitingEventFlags"
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x50U))
        printf "osRtxThreadWaitingMutex"
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x60U))
        printf "osRtxThreadWaitingSemaphore"
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x70U))
        printf "osRtxThreadWaitingMemoryPool"
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x80U))
        printf "osRtxThreadWaitingMessageGet"
    end
    if ($thread_tcb->state == (osThreadBlocked | 0x90U))
        printf "osRtxThreadWaitingMessagePut"
    end

    printf "\n--------------------------------------------------------------------------------\n"

    # Display the callstack for this thread.
    if ($thread_tcb == $thread_run_curr)
        # Switch back to the context for the current thread at the time it stopped in the debugger and dump that stack.
        # The context/callstack stored at last context switch is no longer valid.
        thread
    else
        # Switch to this thread's context and dump its callstack.
        thread $thread_tcb
    end
    printf "\n"
end

document threads
Displays information about all active threads.

It displays information like thread pointer, priority, stack size, state, 
and current callstack.
end




# Command to switch between RTX/mbed-os threads.
define thread
    if ($argc == 0)
        # When no thread is specified in argument list, just restore back to the original callstack from when the
        # debugger stopped.
        thread_restore
        bt
    else
        # Switch to the context of the specified thread.
        # Select frame 0 to enable modification of key registers like pc, lr, xpsr, etc.
        select-frame 0
        if ($thread_swapped == 0)
            # Determine if this processor has a FPU or not.
            if ($thread_has_fpu_isvalid == 0)
                # The CPACR can only enable CP10/CP11 if the FPU is present.
                set var $thread_cpacr=*(unsigned int*)0xE000ED88
                if (($thread_cpacr & (0x5 << 20)) == (0x5 << 20))
                    # The FPU is already enabled.
                    set var $thread_has_fpu=1
                else
                    # Try enabling and see what happens.
                    set var *(unsigned int*)0xE000ED88=$thread_cpacr | (0xF << 20)
                    if ((*(unsigned int*)0xE000ED88 & (0xF << 20)) != (0xF << 20))
                        # CP10 & CP11 didn't enable so no FPU.
                        set var $thread_has_fpu=0
                    else
                        # CP10 & CPP did enable, reset it back to its original state.
                        set var *(unsigned int*)0xE000ED88=$thread_cpacr
                        set var $thread_has_fpu=1
                    end
                end
                set var $thread_has_fpu_isvalid=1
            end
            
            # Save the original callstack from when the debugger stopped.
            set var $thread_r0=(unsigned int)$r0
            set var $thread_r1=(unsigned int)$r1
            set var $thread_r2=(unsigned int)$r2
            set var $thread_r3=(unsigned int)$r3
            set var $thread_r4=(unsigned int)$r4
            set var $thread_r5=(unsigned int)$r5
            set var $thread_r6=(unsigned int)$r6
            set var $thread_r7=(unsigned int)$r7
            set var $thread_r8=(unsigned int)$r8
            set var $thread_r9=(unsigned int)$r9
            set var $thread_r10=(unsigned int)$r10
            set var $thread_r11=(unsigned int)$r11
            set var $thread_r12=(unsigned int)$r12
            set var $thread_sp=(unsigned int)$sp
            set var $thread_lr=(unsigned int)$lr
            set var $thread_pc=(unsigned int)$pc
            set var $thread_xpsr=(unsigned int)$xpsr
            if ($thread_has_fpu == 1)
                set var $thread_s0=(float)$s0
                set var $thread_s1=(float)$s1
                set var $thread_s2=(float)$s2
                set var $thread_s3=(float)$s3
                set var $thread_s4=(float)$s4
                set var $thread_s5=(float)$s5
                set var $thread_s6=(float)$s6
                set var $thread_s7=(float)$s7
                set var $thread_s8=(float)$s8
                set var $thread_s9=(float)$s9
                set var $thread_s10=(float)$s10
                set var $thread_s11=(float)$s11
                set var $thread_s12=(float)$s12
                set var $thread_s13=(float)$s13
                set var $thread_s14=(float)$s14
                set var $thread_s15=(float)$s15
                set var $thread_s16=(float)$s16
                set var $thread_s17=(float)$s17
                set var $thread_s18=(float)$s18
                set var $thread_s19=(float)$s19
                set var $thread_s20=(float)$s20
                set var $thread_s21=(float)$s21
                set var $thread_s22=(float)$s22
                set var $thread_s23=(float)$s23
                set var $thread_s24=(float)$s24
                set var $thread_s25=(float)$s25
                set var $thread_s26=(float)$s26
                set var $thread_s27=(float)$s27
                set var $thread_s28=(float)$s28
                set var $thread_s29=(float)$s29
                set var $thread_s30=(float)$s30
                set var $thread_s31=(float)$s31
                set var $thread_fpscr=(unsigned int)$fpscr
            end
            set var $thread_swapped=1
        end
        # Set current register context to match the saved context state for the specified thread.
        set var $thread_ptr=$arg0
        set var $thread_context=(unsigned int*)((osRtxThread_t*)$thread_ptr)->sp
        set var $thread_float_context=(float*)$thread_context
        if ($thread_has_fpu == 1 && (((osRtxThread_t*)$thread_ptr)->stack_frame & 0x10) == 0)
            set var $thread_off=16
            set var $thread_stacked_count=16+34
        else
            set var $thread_off=0
            set var $thread_stacked_count=16
        end
        set var $pc=$thread_context[$thread_off+14]
        set var $lr=$thread_context[$thread_off+13]
        set var $xpsr=$thread_context[$thread_off+15]
        set var $sp=4*($thread_stacked_count + (($xpsr >> 9) & 1)) + (unsigned int)$thread_context
        set var $r4=$thread_context[$thread_off+0]
        set var $r5=$thread_context[$thread_off+1]
        set var $r6=$thread_context[$thread_off+2]
        set var $r7=$thread_context[$thread_off+3]
        set var $r8=$thread_context[$thread_off+4]
        set var $r9=$thread_context[$thread_off+5]
        set var $r10=$thread_context[$thread_off+6]
        set var $r11=$thread_context[$thread_off+7]
        set var $r0=$thread_context[$thread_off+8]
        set var $r1=$thread_context[$thread_off+9]
        set var $r2=$thread_context[$thread_off+10]
        set var $r3=$thread_context[$thread_off+11]
        set var $r12=$thread_context[$thread_off+12]
        if ($thread_off != 0)
            set var $s16=$thread_float_context[0]
            set var $s17=$thread_float_context[1]
            set var $s18=$thread_float_context[2]
            set var $s19=$thread_float_context[3]
            set var $s20=$thread_float_context[4]
            set var $s21=$thread_float_context[5]
            set var $s22=$thread_float_context[6]
            set var $s23=$thread_float_context[7]
            set var $s24=$thread_float_context[8]
            set var $s25=$thread_float_context[9]
            set var $s26=$thread_float_context[10]
            set var $s27=$thread_float_context[11]
            set var $s28=$thread_float_context[12]
            set var $s29=$thread_float_context[13]
            set var $s30=$thread_float_context[14]
            set var $s31=$thread_float_context[15]
            set var $s0=$thread_float_context[32]
            set var $s1=$thread_float_context[33]
            set var $s2=$thread_float_context[34]
            set var $s3=$thread_float_context[35]
            set var $s4=$thread_float_context[36]
            set var $s5=$thread_float_context[37]
            set var $s6=$thread_float_context[38]
            set var $s7=$thread_float_context[39]
            set var $s8=$thread_float_context[40]
            set var $s9=$thread_float_context[41]
            set var $s10=$thread_float_context[42]
            set var $s11=$thread_float_context[43]
            set var $s12=$thread_float_context[44]
            set var $s13=$thread_float_context[45]
            set var $s14=$thread_float_context[46]
            set var $s15=$thread_float_context[47]
            set var $fpscr=$thread_context[48]
            # offset 49 is a dummy to keep 8-byte alignment.
        end

        # Don't display stack if we have a (any) second parameter.
        if ($argc < 2)
            bt
        end
    end
end

document thread
Switches current thread context.

If an argument is provided, the context will be switched to the thread
with that thread pointer (thread pointers can be seen in "threads" command 
output).  If no argument is provided, it will switch back to the original
context at the time of the stop.
end




# Restores context back to the thread which originally caused the stop into GDB.
# User can invoke by typing "thread" with no arguments.
define thread_restore
    if ($thread_swapped == 1)
        select-frame 0  
        set var $pc=$thread_pc
        set var $lr=$thread_lr
        set var $xpsr=$thread_xpsr
        set var $sp=$thread_sp
        set var $r0=$thread_r0
        set var $r1=$thread_r1
        set var $r2=$thread_r2
        set var $r3=$thread_r3
        set var $r4=$thread_r4
        set var $r5=$thread_r5
        set var $r6=$thread_r6
        set var $r7=$thread_r7
        set var $r8=$thread_r8
        set var $r9=$thread_r9
        set var $r10=$thread_r10
        set var $r11=$thread_r11
        set var $r12=$thread_r12
        if ($thread_has_fpu == 1)
            set var $s0=$thread_s0
            set var $s1=$thread_s1
            set var $s2=$thread_s2
            set var $s3=$thread_s3
            set var $s4=$thread_s4
            set var $s5=$thread_s5
            set var $s6=$thread_s6
            set var $s7=$thread_s7
            set var $s8=$thread_s8
            set var $s9=$thread_s9
            set var $s10=$thread_s10
            set var $s11=$thread_s11
            set var $s12=$thread_s12
            set var $s13=$thread_s13
            set var $s14=$thread_s14
            set var $s15=$thread_s15
            set var $s16=$thread_s16
            set var $s17=$thread_s17
            set var $s18=$thread_s18
            set var $s19=$thread_s19
            set var $s20=$thread_s20
            set var $s21=$thread_s21
            set var $s22=$thread_s22
            set var $s23=$thread_s23
            set var $s24=$thread_s24
            set var $s25=$thread_s25
            set var $s26=$thread_s26
            set var $s27=$thread_s27
            set var $s28=$thread_s28
            set var $s29=$thread_s29
            set var $s30=$thread_s30
            set var $s31=$thread_s31
            set var $fpscr=$thread_fpscr
        end
        set var $thread_swapped=0
    end
end




# Command to dump the current amount of space allocated to the heap.
define heapsize
    set var $heap_base=(((unsigned int)&__bss_end__+7)&~7)
    printf "heap size: %u bytes\n", ('_sbrk_r::heap' - $heap_base)
end

document heapsize
Displays the current heap size.
end




# Command to dump the heap allocations (in-use and free).
define heapwalk
    set var $chunk_curr=(((unsigned int)&__bss_end__+7)&~7)
    set var $chunk_number=1
    set var $used_bytes=(unsigned int)0
    set var $free_bytes=(unsigned int)0
    if (sizeof(struct _reent) == 96)
        # newlib-nano library in use.
        set var $free_curr=(unsigned int)__malloc_free_list
        while ($chunk_curr < '_sbrk_r::heap')
            set var $chunk_size=*(unsigned int*)$chunk_curr
            set var $chunk_next=$chunk_curr + $chunk_size
            if ($chunk_curr == $free_curr)
                set var $chunk_free=1
                set var $free_curr=*(unsigned int*)($free_curr + 4)
            else
                set var $chunk_free=0
            end
            set var $chunk_orig=$chunk_curr + 4
            set var $chunk_curr=($chunk_orig + 7) & ~7
            set var $chunk_size=$chunk_size - 8
            printf "Chunk: %u  Address: 0x%08X  Size: %u  ", $chunk_number, $chunk_curr, $chunk_size
            if ($chunk_free)
                printf "FREE CHUNK"
                set var $free_bytes+=$chunk_size
            else
                set var $used_bytes+=$chunk_size
            end
            printf "\n"
            set var $chunk_curr=$chunk_next
            set var $chunk_number=$chunk_number+1
        end
    else
        # full newlib library in use.
        while ($chunk_curr < '_sbrk_r::heap')
            set var $chunk_size=*(unsigned int*)($chunk_curr + 4)
            set var $chunk_size&=~1
            set var $chunk_next=$chunk_curr + $chunk_size
            set var $chunk_inuse=(*(unsigned int*)($chunk_next + 4)) & 1
    
            # A 0-byte chunk at the beginning of the heap is the initial state before any allocs occur.
            if ($chunk_size == 0)
                loop_break
            end
            
            # The actual data starts past the 8 byte header.
            set var $chunk_orig=$chunk_curr + 8
            # The actual data is 4 bytes smaller than the total chunk since it can use the first word of the next chunk
            # as well since that is its footer.
            set var $chunk_size=$chunk_size - 4
            printf "Chunk: %u  Address: 0x%08X  Size: %u  ", $chunk_number, $chunk_orig, $chunk_size
            if ($chunk_inuse == 0)
                printf "FREE CHUNK"
                set var $free_bytes+=$chunk_size
            else
                set var $used_bytes+=$chunk_size
            end
            printf "\n"
            set var $chunk_curr=$chunk_next
            set var $chunk_number=$chunk_number+1
        end
    end
    printf "  Used bytes: %u\n", $used_bytes
    printf "  Free bytes: %u\n", $free_bytes
end

document heapwalk
Walks the heap and dumps each chunk encountered.
end




# Command to enable/disable catching of Cortex-M faults as soon as they occur.
define catchfaults
    if ($argc > 0 && $arg0==0)
        set var $catch_faults=0
        set var *(int*)0xE000EDFC &= ~0x7F0
    else
        set var $catch_faults=1
        set var *(int*)0xE000EDFC |= 0x7F0
    end
end

document catchfaults
Instructs Cortex-M processor to stop in GDB if any fault is detected.

User can pass in a parameter of 0 to disable this feature. It doesn't work
for Cortex-M0 or Cortex-M0+ devices.
end




# Command to display information about an ARMv7-M fault if one is currently active in the current frame.
define showfault
    set var $ipsr_val = $xpsr & 0xF
    if ($ipsr_val >= 3 && $ipsr_val <= 6)
        # Dump Hard Fault.
        set var $fault_reg = *(unsigned int*)0xE000ED2C
        if ($fault_reg != 0)
            printf "**Hard Fault**\n"
            printf "  Status Register: 0x%08X\n", $fault_reg
            if ($fault_reg & (1 << 31))
                printf "    Debug Event\n"
            end
            if ($fault_reg & (1 << 1))
                printf "    Vector Table Read\n"
            end
            if ($fault_reg & (1 << 30))
                printf "    Forced\n"
            end
        end
    
        set var $cfsr_val = *(unsigned int*)0xE000ED28
    
        # Dump Memory Fault.
        set var $fault_reg = $cfsr_val & 0xFF
        if ($fault_reg != 0)
            printf "**MPU Fault**\n"
            printf "  Status Register: 0x%08X\n", $fault_reg
            if ($fault_reg & (1 << 7))
                printf "    Fault Address: 0x%08X\n", *(unsigned int*)0xE000ED34
            end
            if ($fault_reg & (1 << 5))
                printf "    FP Lazy Preservation\n"
            end
            if ($fault_reg & (1 << 4))
                printf "    Stacking Error\n"
            end
            if ($fault_reg & (1 << 3))
                printf "    Unstacking Error\n"
            end
            if ($fault_reg & (1 << 1))
                printf "    Data Access\n"
            end
            if ($fault_reg & (1 << 0))
                printf "    Instruction Fetch\n"
            end
        end
    
        # Dump Bus Fault.
        set var $fault_reg = ($cfsr_val >> 8) & 0xFF
        if ($fault_reg != 0)
            printf "**Bus Fault**\n"
            printf "  Status Register: 0x%08X\n", $fault_reg
            if ($fault_reg & (1 << 7))
                printf "    Fault Address: 0x%08X\n", *(unsigned int*)0xE000ED38
            end
            if ($fault_reg & (1 << 5))
                printf "    FP Lazy Preservation\n"
            end
            if ($fault_reg & (1 << 4))
                printf "    Stacking Error\n"
            end
            if ($fault_reg & (1 << 3))
                printf "    Unstacking Error\n"
            end
            if ($fault_reg & (1 << 2))
                printf "    Imprecise Data Access\n"
            end
            if ($fault_reg & (1 << 1))
                printf "    Precise Data Access\n"
            end
            if ($fault_reg & (1 << 0))
                printf "    Instruction Prefetch\n"
            end
        end
    
        # Usage Fault.
        set var $fault_reg = $cfsr_val >> 16
        if ($fault_reg != 0)
            printf "**Usage Fault**\n"
            printf "  Status Register: 0x%08X\n", $fault_reg
            if ($fault_reg & (1 << 9))
                printf "    Divide by Zero\n"
            end
            if ($fault_reg & (1 << 8))
                printf "    Unaligned Access\n"
            end
            if ($fault_reg & (1 << 3))
                printf "    Coprocessor Access\n"
            end
            if ($fault_reg & (1 << 2))
                printf "    Invalid Exception Return State\n"
            end
            if ($fault_reg & (1 << 1))
                printf "    Invalid State\n"
            end
            if ($fault_reg & (1 << 0))
                printf "    Undefined Instruction\n"
            end
        end
    else
        printf "Not currently in Cortex-M fault handler!\n"
    end
    
end

document showfault
Display ARMv7-M fault information if current stack frame is in a fault handler.
end




# Some of the stock GDB commands have to be hooked to properly handle that we may have switched to a different
# thread's context and need to restore back to the original state.
define hook-continue
    if ($thread_swapped == 1)
        thread_restore
    end
    if ($catch_faults == 1)
        set var *(int*)0xE000EDFC |= 0x7F0
    end
end

define hook-step
    if ($thread_swapped == 1)
        thread_restore
    end
end

define hook-quit
    if ($thread_swapped == 1)
        thread_restore
    end
end


# I always want to reset the device after a load.
define hookpost-load
    monitor reset
end




# Default to enabling fault catching when user issues the continue execution command.
set var $catch_faults=1

# These variables track whether we have swapped to a different thread's context.
set var $thread_swapped=0
set var $thread_ptr=0

# Haven't determined if CPU has FPU state that could be stored in thread context yet.
set var $thread_has_fpu_isvalid=0

# Settings that I just find work the best for embedded/remote target debugging:
#   No need to ask if an invalid symbol name should be postponed until something gets loaded later...it won't!
set breakpoint pending off
#   Defaults to UTF-8 and some garbage strings cause GDB to hang when it attempts to walk the string.
set target-charset ASCII
#   Just makes structures look a bit nicer.
set print pretty on
#   Can't access peripheral registers and some regions of RAM without this turned off.
set mem inaccessible-by-default off
#   Show disassembly on each stop. Makes it easier to switch to using stepi/nexti when debugging optimized code.
# set disassemble-next-line on
#   Never just set ambiguous breakpoint at multiple locations. Ask which to actually use. We have limited breakpoints
#   on embedded targets.
set multiple-symbols ask
#   Save command history on exit.
set history save on

# UNDONE: Doing this because GDB goes off in the weeds due to how osThreadExit() is placed on thread stack.
set backtrace limit 64
