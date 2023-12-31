#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <stdint.h>
#include <semaphore.h>


typedef uint64_t AccountNumber;
typedef int64_t AccountAmount;

typedef struct Account {
  sem_t lock_balance;
  sem_t lock_account;
  sem_t lock_adjust;
  
  AccountNumber accountNumber;
  AccountAmount balance;
} Account;


Account *Account_LookupByNumber(struct Bank *bank, AccountNumber accountNum);

void Account_Adjust(struct Bank *bank, Account *account,
                    AccountAmount amount,
                    int updateBranch);

AccountAmount Account_Balance(Account *account);

AccountNumber Account_MakeAccountNum(int branch, int subaccount);

int Account_IsSameBranch(AccountNumber accountNum1, AccountNumber accountNum2);

void Account_Init(Bank *bank, Account *account, int id, int branch,
                  AccountAmount initialAmount);

#endif /* _ACCOUNT_H */
