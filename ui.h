#ifndef UI_H
#define UI_H

#include <memory>
#include <vector>

#include "auth.h"
#include "bank.h"
#include "models.h"

void printAccountLine(Bank& bank, const std::shared_ptr<Account>& account);
void showTransactions(const std::vector<Transaction>& txs);
void customerMenu(Bank& bank, AuthService& auth);
void adminMenu(Bank& bank, AuthService& auth);

#endif
