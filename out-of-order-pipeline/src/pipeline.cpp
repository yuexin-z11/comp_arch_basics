//////////////////////////////////////////////////////////////////////
// In part B, you must modify this file to implement the following: //
// - void pipe_cycle_issue(Pipeline *p)                             //
// - void pipe_cycle_schedule(Pipeline *p)                          //
// - void pipe_cycle_writeback(Pipeline *p)                         //
// - void pipe_cycle_commit(Pipeline *p)                            //
//////////////////////////////////////////////////////////////////////

// pipeline.cpp
// Implements the out-of-order pipeline.

#include "pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * The width of the pipeline; that is, the maximum number of instructions that
 * can be processed during any given cycle in each of the issue, schedule, and
 * commit stages of the pipeline.
 *
 * (Note that this does not apply to the writeback stage: as many as
 * MAX_WRITEBACKS instructions can be written back to the ROB in a single
 * cycle!)
 *
 * When the width is 1, the pipeline is scalar.
 * When the width is greater than 1, the pipeline is superscalar.
 */
extern uint32_t PIPE_WIDTH;

/**
 * The number of entries in the ROB; that is, the maximum number of
 * instructions that can be stored in the ROB at any given time.
 *
 * You should use only this many entries of the ROB::entries array.
 */
extern uint32_t NUM_ROB_ENTRIES;

/**
 * Whether to use in-order scheduling or out-of-order scheduling.
 *
 * The possible values are SCHED_IN_ORDER for in-order scheduling and
 * SCHED_OUT_OF_ORDER for out-of-order scheduling.
 *
 * Your implementation of pipe_cycle_sched() should check this value and
 * implement scheduling of instructions accordingly.
 */
extern SchedulingPolicy SCHED_POLICY;

/**
 * The number of cycles an LD instruction should take to execute.
 *
 * This is used by the code in exeq.cpp to determine how long to wait before
 * considering the execution of an LD instruction done.
 */
extern uint32_t LOAD_EXE_CYCLES;

/**
 * Read a single trace record from the trace file and use it to populate the
 * given fe_latch.
 *
 * You should not modify this function.
 *
 * @param p the pipeline whose trace file should be read
 * @param fe_latch the PipelineLatch struct to populate
 */
void pipe_fetch_inst(Pipeline *p, PipelineLatch *fe_latch)
{
    InstInfo *inst = &fe_latch->inst;
    TraceRec trace_rec;
    uint8_t *trace_rec_buf = (uint8_t *)&trace_rec;
    size_t bytes_read_total = 0;
    ssize_t bytes_read_last = 0;
    size_t bytes_left = sizeof(TraceRec);

    // Read a total of sizeof(TraceRec) bytes from the trace file.
    while (bytes_left > 0)
    {
        bytes_read_last = read(p->trace_fd, trace_rec_buf, bytes_left);
        if (bytes_read_last <= 0)
        {
            // EOF or error
            break;
        }

        trace_rec_buf += bytes_read_last;
        bytes_read_total += bytes_read_last;
        bytes_left -= bytes_read_last;
    }

    // Check for error conditions.
    if (bytes_left > 0 || trace_rec.op_type >= NUM_OP_TYPES)
    {
        fe_latch->valid = false;
        p->halt_inst_num = p->last_inst_num;

        if (p->stat_retired_inst >= p->halt_inst_num)
        {
            p->halt = true;
        }

        if (bytes_read_last == -1)
        {
            fprintf(stderr, "\n");
            perror("Couldn't read from pipe");
            return;
        }

        if (bytes_read_total == 0)
        {
            // No more trace records to read
            return;
        }

        // Too few bytes read or invalid op_type
        fprintf(stderr, "\n");
        fprintf(stderr, "Error: Invalid trace file\n");
        return;
    }

    // Got a valid trace record!
    fe_latch->valid = true;
    fe_latch->stall = false;
    inst->inst_num = ++p->last_inst_num;
    inst->op_type = (OpType)trace_rec.op_type;

    inst->dest_reg = trace_rec.dest_needed ? trace_rec.dest_reg : -1;
    inst->src1_reg = trace_rec.src1_needed ? trace_rec.src1_reg : -1;
    inst->src2_reg = trace_rec.src2_needed ? trace_rec.src2_reg : -1;

    inst->dr_tag = -1;
    inst->src1_tag = -1;
    inst->src2_tag = -1;
    inst->src1_ready = false;
    inst->src2_ready = false;
    inst->exe_wait_cycles = 0;
}

/**
 * Allocate and initialize a new pipeline.
 *
 * You should not need to modify this function.
 *
 * @param trace_fd the file descriptor from which to read trace records
 * @return a pointer to a newly allocated pipeline
 */
Pipeline *pipe_init(int trace_fd)
{
    printf("\n** PIPELINE IS %d WIDE **\n\n", PIPE_WIDTH);

    // Allocate pipeline.
    Pipeline *p = (Pipeline *)calloc(1, sizeof(Pipeline));

    // Initialize pipeline.
    p->rat = rat_init();
    p->rob = rob_init();
    p->exeq = exeq_init();
    p->trace_fd = trace_fd;
    p->halt_inst_num = (uint64_t)(-1) - 3;

    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        p->FE_latch[i].valid = false;
        p->ID_latch[i].valid = false;
        p->SC_latch[i].valid = false;
    }
    for (unsigned int i = 0; i < MAX_WRITEBACKS; i++)
    {
        p->EX_latch[i].valid = false;
    }

    return p;
}

/**
 * Commit the given instruction.
 *
 * This updates counters and flags on the pipeline.
 *
 * This function is implemented for you. You should not modify it.
 *
 * @param p the pipeline to update.
 * @param inst the instruction to commit.
 */
void pipe_commit_inst(Pipeline *p, InstInfo inst)
{
    p->stat_retired_inst++;

    if (inst.inst_num >= p->halt_inst_num)
    {
        p->halt = true;
    }
}

/**
 * Print out the state of the pipeline for debugging purposes.
 *
 * You may use this function to help debug your pipeline implementation, but
 * please remove calls to this function before submitting the lab.
 *
 * @param p the pipeline
 */
void pipe_print_state(Pipeline *p)
{
    printf("\n--------------------------------------------\n");
    printf("Cycle count: %lu, retired instructions: %lu\n",
           (unsigned long)p->stat_num_cycle,
           (unsigned long)p->stat_retired_inst);

    // Print table header
    for (unsigned int latch_type = 0; latch_type < 4; latch_type++)
    {
        switch (latch_type)
        {
        case 0:
            printf(" FE:    ");
            break;
        case 1:
            printf(" ID:    ");
            break;
        case 2:
            printf(" SCH:   ");
            break;
        case 3:
            printf(" EX:    ");
            break;
        default:
            printf(" ------ ");
        }
    }
    printf("\n");

    // Print row for each lane in pipeline width
    unsigned int ex_i = 0;
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (p->FE_latch[i].valid)
        {
            printf(" %6lu ",
                   (unsigned long)p->FE_latch[i].inst.inst_num);
        }
        else
        {
            printf(" ------ ");
        }
        if (p->ID_latch[i].valid)
        {
            printf(" %6lu ",
                   (unsigned long)p->ID_latch[i].inst.inst_num);
        }
        else
        {
            printf(" ------ ");
        }
        if (p->SC_latch[i].valid)
        {
            printf(" %6lu ",
                   (unsigned long)p->SC_latch[i].inst.inst_num);
        }
        else
        {
            printf(" ------ ");
        }
        for (; ex_i < MAX_WRITEBACKS; ex_i++)
        {
            if (p->EX_latch[ex_i].valid)
            {
                printf(" %6lu ",
                       (unsigned long)p->EX_latch[ex_i].inst.inst_num);
                ex_i++;
                break;
            }
        }
        printf("\n");
    }
    printf("\n");

    rat_print_state(p->rat);
    exeq_print_state(p->exeq);
    rob_print_state(p->rob);
}

/**
 * Simulate one cycle of all stages of a pipeline.
 *
 * You should not need to modify this function except for debugging purposes.
 * If you add code to print debug output in this function, remove it or comment
 * it out before you submit the lab.
 *
 * @param p the pipeline to simulate
 */
void pipe_cycle(Pipeline *p)
{
    p->stat_num_cycle++;

    // In our simulator, stages are processed in reverse order.
    pipe_cycle_commit(p);
    pipe_cycle_writeback(p);
    pipe_cycle_exe(p);
    pipe_cycle_schedule(p);
    pipe_cycle_issue(p);
    pipe_cycle_decode(p);
    pipe_cycle_fetch(p);

    // You can uncomment the following line to print out the pipeline state
    // after each clock cycle for debugging purposes.
    // Make sure you comment it out or remove it before you submit the lab.
    //pipe_print_state(p);
}

/**
 * Simulate one cycle of the fetch stage of a pipeline.
 *
 * This function is implemented for you. You should not modify it.
 *
 * @param p the pipeline to simulate
 */
void pipe_cycle_fetch(Pipeline *p)
{
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (!p->FE_latch[i].stall && !p->FE_latch[i].valid)
        {
            // No stall and latch empty, so fetch a new instruction.
            pipe_fetch_inst(p, &p->FE_latch[i]);
        }
    }
}

/**
 * Simulate one cycle of the instruction decode stage of a pipeline.
 *
 * This function is implemented for you. You should not modify it.
 *
 * @param p the pipeline to simulate
 */
void pipe_cycle_decode(Pipeline *p)
{
    static uint64_t next_inst_num = 1;
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (!p->ID_latch[i].stall && !p->ID_latch[i].valid)
        {
            // No stall and latch empty, so decode the next instruction.
            // Loop to find the next in-order instruction.
            for (unsigned int j = 0; j < PIPE_WIDTH; j++)
            {
                if (p->FE_latch[j].valid &&
                    p->FE_latch[j].inst.inst_num == next_inst_num)
                {
                    p->ID_latch[i] = p->FE_latch[j];
                    p->FE_latch[j].valid = false;
                    next_inst_num++;
                    break;
                }
            }
        }
    }
}

/**
 * Simulate one cycle of the execute stage of a pipeline. This handles
 * instructions that take multiple cycles to execute.
 *
 * This function is implemented for you. You should not modify it.
 *
 * @param p the pipeline to simulate
 */
void pipe_cycle_exe(Pipeline *p)
{
    // If all operations are single-cycle, just copy SC latches to EX latches.
    if (LOAD_EXE_CYCLES == 1)
    {
        for (unsigned int i = 0; i < PIPE_WIDTH; i++)
        {
            if (p->SC_latch[i].valid)
            {
                p->EX_latch[i] = p->SC_latch[i];
                p->SC_latch[i].valid = false;
            }
        }
        return;
    }

    // Otherwise, we need to handle multi-cycle instructions with EXEQ.

    // All valid entries from the SC latches are inserted into the EXEQ.
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (p->SC_latch[i].valid)
        {
            if (!exeq_insert(p->exeq, p->SC_latch[i].inst))
            {
                fprintf(stderr, "Error: EXEQ full\n");
                p->halt = true;
                return;
            }

            p->SC_latch[i].valid = false;
        }
    }

    // Cycle the EXEQ to reduce wait time for each instruction by 1 cycle.
    exeq_cycle(p->exeq);

    // Transfer all finished entries from the EXEQ to the EX latch.
    for (unsigned int i = 0; i < MAX_WRITEBACKS && exeq_check_done(p->exeq); i++)
    {
        p->EX_latch[i].valid = true;
        p->EX_latch[i].stall = false;
        p->EX_latch[i].inst = exeq_remove(p->exeq);
    }
}

/**
 * Simulate one cycle of the issue stage of a pipeline: insert decoded
 * instructions into the ROB and perform register renaming.
 *
 * You must implement this function in pipeline.cpp in part B of the
 * assignment.
 *
 * @param p the pipeline to simulate
 */
void pipe_cycle_issue(Pipeline *p)
{
    for (unsigned int issued = 0; issued < PIPE_WIDTH; issued++)
    {
        PipelineLatch *latch = NULL;
        for (unsigned int i = 0; i < PIPE_WIDTH; i++)
        {
            PipelineLatch *candidate = &p->ID_latch[i];
            if (candidate->valid &&
                (latch == NULL || candidate->inst.inst_num < latch->inst.inst_num))
                latch = candidate;
        }
        if (latch == NULL)
            break;
        if (!rob_check_space(p->rob))
        {
            latch->stall = true;
            break;
        }

        InstInfo inst = latch->inst;
        int src1 = inst.src1_reg >= 0 ? rat_get_remap(p->rat, inst.src1_reg) : -1;
        int src2 = inst.src2_reg >= 0 ? rat_get_remap(p->rat, inst.src2_reg) : -1;
        inst.src1_ready = src1 < 0 || rob_check_ready(p->rob, src1);
        inst.src2_ready = src2 < 0 || rob_check_ready(p->rob, src2);
        inst.src1_tag = inst.src1_ready ? -1 : src1;
        inst.src2_tag = inst.src2_ready ? -1 : src2;
        inst.dr_tag = p->rob->tail_ptr;

        rob_insert(p->rob, inst);
        if (inst.dest_reg >= 0)
            rat_set_remap(p->rat, inst.dest_reg, inst.dr_tag);
        latch->valid = false;
        latch->stall = false;
    }
}

/**
 * Simulate one cycle of the scheduling stage of a pipeline: schedule
 * instructions to execute if they are ready.
 *
 * You must implement this function in pipeline.cpp in part B of the
 * assignment.
 *
 * @param p the pipeline to simulate
 */
void pipe_cycle_schedule(Pipeline *p)
{
    for (unsigned int lane = 0; lane < PIPE_WIDTH; lane++)
    {
        if (p->SC_latch[lane].valid)
            continue;

        ROBEntry *selected = NULL;
        for (unsigned int offset = 0; offset < NUM_ROB_ENTRIES; offset++)
        {
            unsigned int index = (p->rob->head_ptr + offset) % NUM_ROB_ENTRIES;
            ROBEntry *entry = &p->rob->entries[index];
            if (!entry->valid || entry->exec)
                continue;
            if (entry->inst.src1_ready && entry->inst.src2_ready)
            {
                selected = entry;
                break;
            }
            if (SCHED_POLICY == SCHED_IN_ORDER)
                return;
        }

        if (selected == NULL)
            return;
        rob_mark_exec(p->rob, selected->inst);
        p->SC_latch[lane].inst = selected->inst;
        p->SC_latch[lane].valid = true;
        p->SC_latch[lane].stall = false;
    }
}

/**
 * Simulate one cycle of the writeback stage of a pipeline: update the ROB
 * with information from instructions that have finished executing.
 *
 * You must implement this function in pipeline.cpp in part B of the
 * assignment.
 *
 * @param p the pipeline to simulate
 */
void pipe_cycle_writeback(Pipeline *p)
{
    for (unsigned int i = 0; i < MAX_WRITEBACKS; i++)
    {
        PipelineLatch *latch = &p->EX_latch[i];
        if (!latch->valid)
            continue;
        rob_wakeup(p->rob, latch->inst.dr_tag);
        rob_mark_ready(p->rob, latch->inst);
        latch->valid = false;
    }
}

/**
 * Simulate one cycle of the commit stage of a pipeline: commit instructions
 * in the ROB that are ready to commit.
 *
 * You must implement this function in pipeline.cpp in part B of the
 * assignment.
 *
 * @param p the pipeline to simulate
 */
void pipe_cycle_commit(Pipeline *p)
{
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (!rob_check_head(p->rob))
            break;
        InstInfo inst = rob_remove_head(p->rob);
        pipe_commit_inst(p, inst);
        if (inst.dest_reg >= 0 &&
            rat_get_remap(p->rat, inst.dest_reg) == inst.dr_tag)
            rat_reset_entry(p->rat, inst.dest_reg);
    }
}
