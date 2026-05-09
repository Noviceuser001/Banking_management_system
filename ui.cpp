#include "ui.h"

#include <iomanip>
#include <iostream>

#include "utils.h"

void printAccountLine(Bank& bank, const std::shared_ptr<Account>& account) {
    const auto customer = bank.findCustomerById(account->getCustomerId());
    std::cout << "Account ID: " << account->getAccountId() << " | Name: "
              << (customer ? customer->getName() : "Unknown") << " | Type: " << account->getType()
              << " | Balance: " << std::fixed << std::setprecision(2) << account->getBalance() << " | Frozen: "
              << (account->isFrozen() ? "Yes" : "No") << '\n';
}

void showTransactions(const std::vector<Transaction>& txs) {
    if (txs.empty()) {
        std::cout << "No transactions found.\n";
        return;
    }

    for (const auto& tx : txs) {
        std::cout << "#" << tx.getTxId() << " | " << tx.getTimestamp() << " | " << tx.getType()
                  << " | From: " << tx.getFromAccount() << " | To: " << tx.getToAccount()
                  << " | Amount: " << std::fixed << std::setprecision(2) << tx.getAmount() << " | Note: "
                  << tx.getNote() << '\n';
    }
}

void customerMenu(Bank& bank, AuthService& auth) {
    while (auth.getRole() == AuthService::Role::Customer) {
        const int accountId = auth.getActiveAccountId();
        auto account = bank.findAccountById(accountId);
        if (!account) {
            std::cout << "Account not found. Logging out.\n";
            auth.logout();
            return;
        }

        std::cout << "\n--- Customer Menu (Account " << accountId << ") ---\n"
                  << "1. Cash Deposit\n"
                  << "2. Cash Withdrawal\n"
                  << "3. Fund Transfer\n"
                  << "4. Balance Enquiry\n"
                  << "5. Transaction History\n"
                  << "6. Mini Statement\n"
                  << "7. Profile Update\n"
                  << "8. Interest Calculation\n"
                  << "9. Account Closure Request\n"
                  << "10. Logout\n";

        const int choice = readInt("Select option: ");

        switch (choice) {
        case 1: {
            const double amount = readDouble("Enter amount to deposit: ");
            if (bank.deposit(accountId, amount)) {
                std::cout << "Deposit successful.\n";
            } else {
                std::cout << "Deposit failed. Check amount/account status.\n";
            }
            break;
        }
        case 2: {
            const double amount = readDouble("Enter amount to withdraw: ");
            if (bank.withdraw(accountId, amount)) {
                std::cout << "Withdrawal successful.\n";
            } else {
                std::cout << "Withdrawal failed (insufficient funds/overdraft limit/frozen account).\n";
            }
            break;
        }
        case 3: {
            const int to = readInt("Enter destination account ID: ");
            const double amount = readDouble("Enter amount to transfer: ");
            if (bank.transfer(accountId, to, amount)) {
                std::cout << "Transfer successful.\n";
            } else {
                std::cout << "Transfer failed.\n";
            }
            break;
        }
        case 4:
            std::cout << "Current Balance: " << std::fixed << std::setprecision(2) << account->getBalance() << "\n";
            break;
        case 5:
            showTransactions(bank.getTransactionsForAccount(accountId));
            break;
        case 6: {
            auto txs = bank.getTransactionsForAccount(accountId);
            if (txs.size() > MINI_STATEMENT_SIZE) {
                txs.erase(txs.begin(), txs.begin() + static_cast<std::ptrdiff_t>(txs.size() - MINI_STATEMENT_SIZE));
            }
            showTransactions(txs);
            break;
        }
        case 7: {
            const std::string name = readLine("New name: ");
            const std::string email = readLine("New email: ");
            const std::string phone = readLine("New phone: ");
            if (bank.updateProfile(accountId, name, email, phone)) {
                std::cout << "Profile updated.\n";
            } else {
                std::cout << "Profile update failed.\n";
            }
            break;
        }
        case 8:
            if (bank.applyInterest(accountId)) {
                std::cout << "Interest applied successfully.\n";
            } else {
                std::cout << "No interest applied (zero/negative balance or frozen account).\n";
            }
            break;
        case 9:
            if (bank.requestClosure(accountId)) {
                std::cout << "Account closure request submitted.\n";
            } else {
                std::cout << "Unable to submit closure request.\n";
            }
            break;
        case 10:
            auth.logout();
            std::cout << "Logged out successfully.\n";
            break;
        default:
            std::cout << "Invalid option.\n";
            break;
        }
    }
}

void adminMenu(Bank& bank, AuthService& auth) {
    while (auth.getRole() == AuthService::Role::Admin) {
        std::cout << "\n--- Admin Menu (" << auth.getActiveAdmin() << ") ---\n"
                  << "1. View All Accounts\n"
                  << "2. Search Account (ID/Name)\n"
                  << "3. Freeze Account\n"
                  << "4. Unfreeze Account\n"
                  << "5. Delete Account\n"
                  << "6. View Audit Log\n"
                  << "7. Generate Reports\n"
                  << "8. Manage Interest Rates\n"
                  << "9. View Closure Requests\n"
                  << "10. Logout\n";

        const int choice = readInt("Select option: ");

        switch (choice) {
        case 1:
            for (const auto& account : bank.getAllAccounts()) {
                printAccountLine(bank, account);
            }
            break;
        case 2: {
            const std::string query = readLine("Enter account ID or customer name: ");
            const auto matches = bank.searchAccounts(query);
            if (matches.empty()) {
                std::cout << "No matching accounts found.\n";
            } else {
                for (const auto& account : matches) {
                    printAccountLine(bank, account);
                }
            }
            break;
        }
        case 3: {
            const int id = readInt("Enter account ID to freeze: ");
            std::cout << (bank.freezeAccount(id, true) ? "Account frozen.\n" : "Failed to freeze account.\n");
            break;
        }
        case 4: {
            const int id = readInt("Enter account ID to unfreeze: ");
            std::cout << (bank.freezeAccount(id, false) ? "Account unfrozen.\n" : "Failed to unfreeze account.\n");
            break;
        }
        case 5: {
            const int id = readInt("Enter account ID to delete: ");
            std::cout << (bank.deleteAccount(id) ? "Account deleted.\n" : "Failed to delete account.\n");
            break;
        }
        case 6: {
            const auto& logs = bank.getAuditLog();
            if (logs.empty()) {
                std::cout << "Audit log is empty.\n";
            } else {
                for (const auto& line : logs) {
                    std::cout << line << '\n';
                }
            }
            break;
        }
        case 7:
            bank.generateReport();
            break;
        case 8: {
            const double savings = readDouble("Enter new savings interest rate (e.g. 0.04): ");
            const double current = readDouble("Enter new current interest rate (e.g. 0.01): ");
            std::cout << (bank.setInterestRates(savings, current) ? "Interest rates updated.\n"
                                                                  : "Invalid rates. Update failed.\n");
            break;
        }
        case 9: {
            const auto requests = bank.getClosureRequests();
            if (requests.empty()) {
                std::cout << "No pending closure requests.\n";
            } else {
                for (const auto& c : requests) {
                    std::cout << "Customer: " << c->getName() << " | Account: " << c->getAccountId() << '\n';
                }
            }
            break;
        }
        case 10:
            auth.logout();
            std::cout << "Admin logged out.\n";
            break;
        default:
            std::cout << "Invalid option.\n";
            break;
        }
    }
}
