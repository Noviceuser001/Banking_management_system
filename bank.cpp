#include "bank.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "utils.h"

Bank::Bank() { loadAll(); }

void Bank::addAudit(const std::string& message) { auditLog_.push_back(nowTimestamp() + " | " + message); }

const std::vector<std::string>& Bank::getAuditLog() const { return auditLog_; }
const std::vector<std::shared_ptr<Account>>& Bank::getAllAccounts() const { return accounts_; }
const std::vector<std::shared_ptr<Customer>>& Bank::getAllCustomers() const { return customers_; }
const std::vector<std::shared_ptr<Admin>>& Bank::getAllAdmins() const { return admins_; }

std::shared_ptr<Account> Bank::findAccountById(int accountId) {
    for (const auto& account : accounts_) {
        if (account->getAccountId() == accountId) {
            return account;
        }
    }
    return nullptr;
}

std::shared_ptr<Customer> Bank::findCustomerById(int customerId) {
    for (const auto& customer : customers_) {
        if (customer->getId() == customerId) {
            return customer;
        }
    }
    return nullptr;
}

std::shared_ptr<Customer> Bank::findCustomerByAccountId(int accountId) {
    for (const auto& customer : customers_) {
        if (customer->getAccountId() == accountId) {
            return customer;
        }
    }
    return nullptr;
}

std::shared_ptr<Admin> Bank::findAdminByUsername(const std::string& username) {
    const std::string key = toLower(username);
    for (const auto& admin : admins_) {
        if (toLower(admin->getUsername()) == key) {
            return admin;
        }
    }
    return nullptr;
}

std::optional<int> Bank::registerCustomer(const std::string& name, const std::string& email, const std::string& phone,
                                          const std::string& pin, const std::string& accountType,
                                          double initialDeposit) {
    if (name.empty() || email.empty() || phone.empty() || pin.empty() || initialDeposit < 0.0) {
        return std::nullopt;
    }

    const int customerId = nextCustomerId_++;
    const int accountId = nextAccountId_++;

    std::shared_ptr<Account> account;
    if (toLower(accountType) == "current") {
        account = std::make_shared<CurrentAccount>(accountId, customerId, 0.0, false);
    } else {
        account = std::make_shared<SavingsAccount>(accountId, customerId, 0.0, false);
    }

    auto customer =
        std::make_shared<Customer>(customerId, name, email, phone, accountId, hashPinBasic(pin), false);

    accounts_.push_back(account);
    customers_.push_back(customer);

    if (initialDeposit > 0.0) {
        deposit(accountId, initialDeposit, "Initial deposit");
    }

    addAudit("Registered customer " + sanitizeField(name) + " with account " + std::to_string(accountId));
    return accountId;
}

bool Bank::deposit(int accountId, double amount, const std::string& note, bool createTx) {
    auto account = findAccountById(accountId);
    if (!account || account->isFrozen() || amount <= 0.0) {
        return false;
    }

    account->setBalance(account->getBalance() + amount);

    if (createTx) {
        transactions_.emplace_back(nextTransactionId_++, nowTimestamp(), "DEPOSIT", accountId, accountId, amount,
                                   sanitizeField(note));
        addAudit("Deposit " + std::to_string(amount) + " to account " + std::to_string(accountId));
    }
    return true;
}

bool Bank::withdraw(int accountId, double amount, const std::string& note, bool createTx) {
    auto account = findAccountById(accountId);
    if (!account || account->isFrozen() || amount <= 0.0 || !account->canWithdraw(amount)) {
        return false;
    }

    account->setBalance(account->getBalance() - amount);

    if (createTx) {
        transactions_.emplace_back(nextTransactionId_++, nowTimestamp(), "WITHDRAW", accountId, accountId, amount,
                                   sanitizeField(note));
        addAudit("Withdraw " + std::to_string(amount) + " from account " + std::to_string(accountId));
    }
    return true;
}

bool Bank::transfer(int fromAccount, int toAccount, double amount) {
    if (fromAccount == toAccount || amount <= 0.0) {
        return false;
    }

    auto src = findAccountById(fromAccount);
    auto dst = findAccountById(toAccount);
    if (!src || !dst || src->isFrozen() || dst->isFrozen()) {
        return false;
    }

    if (!withdraw(fromAccount, amount, "Transfer to " + std::to_string(toAccount), false)) {
        return false;
    }

    if (!deposit(toAccount, amount, "Transfer from " + std::to_string(fromAccount), false)) {
        src->setBalance(src->getBalance() + amount);
        addAudit("Transfer rollback applied for source account " + std::to_string(fromAccount));
        return false;
    }

    transactions_.emplace_back(nextTransactionId_++, nowTimestamp(), "TRANSFER", fromAccount, toAccount, amount,
                               "Fund transfer");
    addAudit("Transfer " + std::to_string(amount) + " from " + std::to_string(fromAccount) + " to " +
             std::to_string(toAccount));
    return true;
}

bool Bank::updateProfile(int accountId, const std::string& name, const std::string& email, const std::string& phone) {
    auto customer = findCustomerByAccountId(accountId);
    if (!customer) {
        return false;
    }
    customer->updateProfile(name, email, phone);
    addAudit("Profile updated for account " + std::to_string(accountId));
    return true;
}

bool Bank::requestClosure(int accountId) {
    auto customer = findCustomerByAccountId(accountId);
    if (!customer) {
        return false;
    }
    customer->setClosureRequested(true);
    addAudit("Closure requested for account " + std::to_string(accountId));
    return true;
}

bool Bank::applyInterest(int accountId) {
    auto account = findAccountById(accountId);
    if (!account || account->isFrozen()) {
        return false;
    }

    const double rate = (account->getType() == "Savings") ? savingsInterestRate_ : currentInterestRate_;
    const double earned = account->applyInterest(rate);
    if (earned <= 0.0) {
        return false;
    }

    transactions_.emplace_back(nextTransactionId_++, nowTimestamp(), "INTEREST", accountId, accountId, earned,
                               "Interest credited");
    addAudit("Interest " + std::to_string(earned) + " applied to account " + std::to_string(accountId));
    return true;
}

std::vector<Transaction> Bank::getTransactionsForAccount(int accountId) const {
    std::vector<Transaction> result;
    for (const auto& tx : transactions_) {
        if (tx.getFromAccount() == accountId || tx.getToAccount() == accountId) {
            result.push_back(tx);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Account>> Bank::searchAccounts(const std::string& query) {
    std::vector<std::shared_ptr<Account>> result;

    bool numeric = !query.empty();
    for (char c : query) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            numeric = false;
            break;
        }
    }

    if (numeric) {
        try {
            const int id = std::stoi(query);
            auto account = findAccountById(id);
            if (account) {
                result.push_back(account);
            }
        } catch (...) {
            return result;
        }
        return result;
    }

    const std::string needle = toLower(query);
    for (const auto& customer : customers_) {
        if (toLower(customer->getName()).find(needle) != std::string::npos) {
            auto account = findAccountById(customer->getAccountId());
            if (account) {
                result.push_back(account);
            }
        }
    }
    return result;
}

bool Bank::freezeAccount(int accountId, bool frozen) {
    auto account = findAccountById(accountId);
    if (!account) {
        return false;
    }
    account->setFrozen(frozen);
    addAudit(std::string(frozen ? "Frozen " : "Unfrozen ") + "account " + std::to_string(accountId));
    return true;
}

bool Bank::deleteAccount(int accountId) {
    auto accIt = std::remove_if(accounts_.begin(), accounts_.end(),
                                [accountId](const std::shared_ptr<Account>& a) {
                                    return a->getAccountId() == accountId;
                                });
    if (accIt == accounts_.end()) {
        return false;
    }
    accounts_.erase(accIt, accounts_.end());

    auto custIt = std::remove_if(customers_.begin(), customers_.end(),
                                 [accountId](const std::shared_ptr<Customer>& c) {
                                     return c->getAccountId() == accountId;
                                 });
    customers_.erase(custIt, customers_.end());

    addAudit("Deleted account " + std::to_string(accountId));
    return true;
}

double Bank::getSavingsInterestRate() const { return savingsInterestRate_; }
double Bank::getCurrentInterestRate() const { return currentInterestRate_; }

bool Bank::setInterestRates(double savingsRate, double currentRate) {
    if (savingsRate < 0.0 || currentRate < 0.0) {
        return false;
    }
    savingsInterestRate_ = savingsRate;
    currentInterestRate_ = currentRate;
    addAudit("Updated interest rates. Savings=" + std::to_string(savingsRate) + " Current=" +
             std::to_string(currentRate));
    return true;
}

std::vector<std::shared_ptr<Customer>> Bank::getClosureRequests() const {
    std::vector<std::shared_ptr<Customer>> requests;
    for (const auto& customer : customers_) {
        if (customer->isClosureRequested()) {
            requests.push_back(customer);
        }
    }
    return requests;
}

void Bank::generateReport() {
    int savingsCount = 0;
    int currentCount = 0;
    double totalBalance = 0.0;

    for (const auto& account : accounts_) {
        totalBalance += account->getBalance();
        if (account->getType() == "Savings") {
            ++savingsCount;
        } else {
            ++currentCount;
        }
    }

    std::cout << "\n--- Report ---\n";
    std::cout << "Total Customers: " << customers_.size() << "\n";
    std::cout << "Total Accounts: " << accounts_.size() << "\n";
    std::cout << "Savings Accounts: " << savingsCount << "\n";
    std::cout << "Current Accounts: " << currentCount << "\n";
    std::cout << "Total Balance Across Accounts: " << std::fixed << std::setprecision(2) << totalBalance << "\n";
    std::cout << "Pending Closure Requests: " << getClosureRequests().size() << "\n";
}

void Bank::loadAll() {
    loadConfig();
    loadAccounts();
    loadCustomers();
    loadAdmins();
    loadTransactions();
    loadAudit();

    if (admins_.empty()) {
        const auto defaultAdmin = std::make_shared<Admin>(nextAdminId_++, "System Admin", "admin@bank.local",
                                                          "N/A", "admin", hashPinBasic("admin123"));
        admins_.push_back(defaultAdmin);
        addAudit("Created default admin (username: admin)");
    }
}

void Bank::saveAll() {
    saveConfig();
    saveAccounts();
    saveCustomers();
    saveAdmins();
    saveTransactions();
    saveAudit();
}

void Bank::loadConfig() {
    std::ifstream in(configFile_);
    if (!in) {
        return;
    }
    std::string line;
    if (std::getline(in, line)) {
        auto p = split(line);
        if (p.size() != 6) {
            addAudit("Invalid config format detected; defaults retained");
            return;
        }
        try {
            nextCustomerId_ = std::stoi(p[0]);
            nextAccountId_ = std::stoi(p[1]);
            nextTransactionId_ = std::stoi(p[2]);
            nextAdminId_ = std::stoi(p[3]);
            savingsInterestRate_ = std::stod(p[4]);
            currentInterestRate_ = std::stod(p[5]);
        } catch (...) {
            addAudit("Malformed config values detected; defaults retained");
        }
    }
}

void Bank::saveConfig() {
    std::ofstream out(configFile_, std::ios::trunc);
    out << nextCustomerId_ << '|' << nextAccountId_ << '|' << nextTransactionId_ << '|' << nextAdminId_ << '|'
        << savingsInterestRate_ << '|' << currentInterestRate_ << '\n';
}

void Bank::loadCustomers() {
    std::ifstream in(customersFile_);
    if (!in) {
        return;
    }

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        const auto p = split(line);
        if (p.size() != 7) {
            addAudit("Skipped malformed customer record at line " + std::to_string(lineNo));
            continue;
        }
        try {
            customers_.push_back(std::make_shared<Customer>(std::stoi(p[0]), p[1], p[2], p[3], std::stoi(p[4]), p[5],
                                                            p[6] == "1"));
        } catch (...) {
            addAudit("Skipped malformed customer record values at line " + std::to_string(lineNo));
        }
    }
}

void Bank::saveCustomers() {
    std::ofstream out(customersFile_, std::ios::trunc);
    for (const auto& c : customers_) {
        out << c->getId() << '|' << sanitizeField(c->getName()) << '|' << sanitizeField(c->getEmail()) << '|'
            << sanitizeField(c->getPhone()) << '|' << c->getAccountId() << '|' << c->getPinHash() << '|'
            << (c->isClosureRequested() ? 1 : 0) << '\n';
    }
}

void Bank::loadAccounts() {
    std::ifstream in(accountsFile_);
    if (!in) {
        return;
    }

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        const auto p = split(line);
        if (p.size() != 5) {
            addAudit("Skipped malformed account record at line " + std::to_string(lineNo));
            continue;
        }
        try {
            const std::string& type = p[0];
            const int accountId = std::stoi(p[1]);
            const int customerId = std::stoi(p[2]);
            const double balance = std::stod(p[3]);
            const bool frozen = p[4] == "1";

            if (type == "Savings") {
                accounts_.push_back(std::make_shared<SavingsAccount>(accountId, customerId, balance, frozen));
            } else if (type == "Current") {
                accounts_.push_back(std::make_shared<CurrentAccount>(accountId, customerId, balance, frozen));
            } else {
                addAudit("Skipped account record with unknown type at line " + std::to_string(lineNo));
            }
        } catch (...) {
            addAudit("Skipped malformed account record values at line " + std::to_string(lineNo));
        }
    }
}

void Bank::saveAccounts() {
    std::ofstream out(accountsFile_, std::ios::trunc);
    for (const auto& a : accounts_) {
        out << a->getType() << '|' << a->getAccountId() << '|' << a->getCustomerId() << '|' << a->getBalance() << '|'
            << (a->isFrozen() ? 1 : 0) << '\n';
    }
}

void Bank::loadAdmins() {
    std::ifstream in(adminsFile_);
    if (!in) {
        return;
    }

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        const auto p = split(line);
        if (p.size() != 6) {
            addAudit("Skipped malformed admin record at line " + std::to_string(lineNo));
            continue;
        }
        try {
            admins_.push_back(std::make_shared<Admin>(std::stoi(p[0]), p[1], p[2], p[3], p[4], p[5]));
        } catch (...) {
            addAudit("Skipped malformed admin record values at line " + std::to_string(lineNo));
        }
    }
}

void Bank::saveAdmins() {
    std::ofstream out(adminsFile_, std::ios::trunc);
    for (const auto& a : admins_) {
        out << a->getId() << '|' << sanitizeField(a->getName()) << '|' << sanitizeField(a->getEmail()) << '|'
            << sanitizeField(a->getPhone()) << '|' << sanitizeField(a->getUsername()) << '|' << a->getPinHash() << '\n';
    }
}

void Bank::loadTransactions() {
    std::ifstream in(transactionsFile_);
    if (!in) {
        return;
    }

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        const auto p = split(line);
        if (p.size() != 7) {
            addAudit("Skipped malformed transaction record at line " + std::to_string(lineNo));
            continue;
        }
        try {
            transactions_.emplace_back(std::stoi(p[0]), p[1], p[2], std::stoi(p[3]), std::stoi(p[4]),
                                       std::stod(p[5]), p[6]);
        } catch (...) {
            addAudit("Skipped malformed transaction record values at line " + std::to_string(lineNo));
        }
    }
}

void Bank::saveTransactions() {
    std::ofstream out(transactionsFile_, std::ios::trunc);
    for (const auto& t : transactions_) {
        out << t.getTxId() << '|' << t.getTimestamp() << '|' << t.getType() << '|' << t.getFromAccount() << '|'
            << t.getToAccount() << '|' << t.getAmount() << '|' << sanitizeField(t.getNote()) << '\n';
    }
}

void Bank::loadAudit() {
    std::ifstream in(auditFile_);
    if (!in) {
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            auditLog_.push_back(line);
        }
    }
}

void Bank::saveAudit() {
    std::ofstream out(auditFile_, std::ios::trunc);
    for (const auto& line : auditLog_) {
        out << line << '\n';
    }
}
