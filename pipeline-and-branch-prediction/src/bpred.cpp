// --------------------------------------------------------------------- //
// For part B, you will need to modify this file.                        //
// You may add any code you need, as long as you correctly implement the //
// three required BPred methods already listed in this file.             //
// --------------------------------------------------------------------- //

// bpred.cpp
// Implements the branch predictor class.

#include "bpred.h"

/**
 * Construct a branch predictor with the given policy.
 * 
 * In part B of the lab, you must implement this constructor.
 * 
 * @param policy the policy this branch predictor should use
 */
BPred::BPred(BPredPolicy policy)
    : policy(policy),
      global_history(0),
      stat_num_branches(0),
      stat_num_mispred(0)
{
    // Start every two-bit counter in the weakly-taken state
    // (binary 10, or decimal 2).
    for (uint32_t i = 0; i < GSHARE_TABLE_SIZE; i++)
    {
        pattern_history_table[i] = 2;
    }
}

/**
 * Get a prediction for the branch with the given address.
 * 
 * In part B of the lab, you must implement this method.
 * 
 * @param pc the address (program counter) of the branch to predict
 * @return the prediction for whether the branch is taken or not taken
 */
BranchDirection BPred::predict(uint64_t pc)
{
    // TODO: Return a prediction for whether the branch at address pc will be
    // TAKEN or NOT_TAKEN according to this branch predictor's policy.
    if (policy == BPRED_ALWAYS_TAKEN)
    {
        return TAKEN;
    }
    else if (policy == BPRED_GSHARE)
    {
        if (pattern_history_table[(pc ^ global_history) & GSHARE_MASK] >= 2)
        {
            return TAKEN;
        }
        else
        {
            return NOT_TAKEN;
        }
    }
    else
    {
        // Note that you do not have to handle the BPRED_PERFECT policy here; this
        // function will not be called for that policy.
        return TAKEN;
    }
}


/**
 * Update the branch predictor statistics (stat_num_branches and
 * stat_num_mispred), as well as any other internal state you may need to
 * update in the branch predictor.
 * 
 * In part B of the lab, you must implement this method.
 * 
 * @param pc the address (program counter) of the branch
 * @param prediction the prediction made by the branch predictor
 * @param resolution the actual outcome of the branch
 */
void BPred::update(uint64_t pc, BranchDirection prediction,
                   BranchDirection resolution)
{
    // TODO: Update the stat_num_branches and stat_num_mispred member variables
    // according to the prediction and resolution of the branch.
    if (prediction != resolution)
    {
        stat_num_mispred++;
    }
    stat_num_branches++;

    // TODO: Update any other internal state you may need to keep track of.
    if (policy == BPRED_GSHARE)
    {
        uint32_t index = (pc ^ global_history) & GSHARE_MASK;
        if (resolution == TAKEN)
        {
            if (pattern_history_table[index] < 3)
            {
                pattern_history_table[index]++;
            }
        }
        else
        {
            if (pattern_history_table[index] > 0)
            {
                pattern_history_table[index]--;
            }
        }
    }

    // update global history register
    if (policy == BPRED_GSHARE)
    {
        global_history = ((global_history << 1) | (resolution == TAKEN ? 1 : 0)) & GSHARE_MASK;
    }

    // Note that you do not have to handle the BPRED_PERFECT policy here; this
    // function will not be called for that policy.
}
