#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
#include "branch.h"

/*
 * deposit money into an account
 */
int
Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  BranchID branch_id = AccountNum_GetBranchID(account->accountNumber);
  sem_wait(&(account->lock_account));
  sem_wait(&(bank->branches[branch_id].lock_branch));
  Account_Adjust(bank,account, amount, 1);
  sem_post(&(account->lock_account));
  sem_post(&(bank->branches[branch_id].lock_branch));
  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int
Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  BranchID branch_id = AccountNum_GetBranchID(account->accountNumber);
  sem_wait(&(account->lock_account));
  sem_wait(&(bank->branches[branch_id].lock_branch));
  if (amount > Account_Balance(account)) {
    sem_post(&(account->lock_account));
    sem_post(&(bank->branches[branch_id].lock_branch));
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank,account, -amount, 1);
  sem_post(&(account->lock_account));
  sem_post(&(bank->branches[branch_id].lock_branch));
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  if (amount > Account_Balance(srcAccount)) {
    return ERROR_INSUFFICIENT_FUNDS;
  }

  if (srcAccountNum == dstAccountNum) { 
    return ERROR_SUCCESS;
  }
  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);
  if (updateBranch) {
    BranchID src_branch = AccountNum_GetBranchID(srcAccountNum);
    BranchID dst_branch = AccountNum_GetBranchID(dstAccountNum);

    if (src_branch > dst_branch) {
      sem_wait(&(dstAccount->lock_account));
      sem_wait(&(srcAccount->lock_account));
      sem_wait(&(bank->branches[dst_branch].lock_branch));
      sem_wait(&(bank->branches[src_branch].lock_branch));
    } else {
      sem_wait(&(srcAccount->lock_account));
      sem_wait(&(dstAccount->lock_account));
      sem_wait(&(bank->branches[src_branch].lock_branch));
      sem_wait(&(bank->branches[dst_branch].lock_branch));
    }  

    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);

    sem_post(&(srcAccount->lock_account));
    sem_post(&(dstAccount->lock_account));
    sem_post(&(bank->branches[dst_branch].lock_branch));
    sem_post(&(bank->branches[src_branch].lock_branch));
    
    return ERROR_SUCCESS;
  }


  if(srcAccountNum > dstAccountNum) {
    sem_wait(&(dstAccount->lock_account));
    sem_wait(&(srcAccount->lock_account));
  } else {
    sem_wait(&(srcAccount->lock_account));
    sem_wait(&(dstAccount->lock_account));  
  }

  Account_Adjust(bank, srcAccount, -amount, updateBranch);
  Account_Adjust(bank, dstAccount, amount, updateBranch);

  sem_post(&(srcAccount->lock_account));
  sem_post(&(dstAccount->lock_account));
  return ERROR_SUCCESS;  
}
