#ifndef BANK_H
#define BANK_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models.h"

class Bank {
  private:
    std::vector<std::shared_ptr<Customer>> customers_;
    std::vector<std::shared_ptr<Account>> accounts_;
    std::vector<std::shared_ptr<Admin>> admins_;
    std::vector<Transaction> transactions_;
    std::vector<std::string> auditLog_;

    int nextCustomerId_ = 1000;
    int nextAccountId_ = 10000;
    int nextTransactionId_ = 1;
    int nextAdminId_ = 1;

    double savingsInterestRate_ = 0.04;
    double currentInterestRate_ = 0.01;

    const std::string customersFile_ = "customers.dat";
    const std::string accountsFile_ = "accounts.dat";
    const std::string adminsFile_ = "admins.dat";
    const std::string transactionsFile_ = "transactions.dat";
    const std::string configFile_ = "config.dat";
    const std::string auditFile_ = "audit.log";

  public:
    Bank();

    void addAudit(const std::string& message);

    const std::vector<std::string>& getAuditLog() const;
    const std::vector<std::shared_ptr<Account>>& getAllAccounts() const;
    const std::vector<std::shared_ptr<Customer>>& getAllCustomers() const;
    const std::vector<std::shared_ptr<Admin>>& getAllAdmins() const;

    std::shared_ptr<Account> findAccountById(int accountId);
    std::shared_ptr<Customer> findCustomerById(int customerId);
    std::shared_ptr<Customer> findCustomerByAccountId(int accountId);
    std::shared_ptr<Admin> findAdminByUsername(const std::string& username);

    std::optional<int> registerCustomer(const std::string& name, const std::string& email, const std::string& phone,
                                        const std::string& pin, const std::string& accountType,
                                        double initialDeposit);

    bool deposit(int accountId, double amount, const std::string& note = "Deposit", bool createTx = true);
    bool withdraw(int accountId, double amount, const std::string& note = "Withdrawal", bool createTx = true);
    bool transfer(int fromAccount, int toAccount, double amount);
    bool updateProfile(int accountId, const std::string& name, const std::string& email, const std::string& phone);
    bool requestClosure(int accountId);
    bool applyInterest(int accountId);

    std::vector<Transaction> getTransactionsForAccount(int accountId) const;
    std::vector<std::shared_ptr<Account>> searchAccounts(const std::string& query);

    bool freezeAccount(int accountId, bool frozen);
    bool deleteAccount(int accountId);

    double getSavingsInterestRate() const;
    double getCurrentInterestRate() const;
    bool setInterestRates(double savingsRate, double currentRate);

    std::vector<std::shared_ptr<Customer>> getClosureRequests() const;
    void generateReport();

    void loadAll();
    void saveAll();

  private:
    void loadConfig();
    void saveConfig();
    void loadCustomers();
    void saveCustomers();
    void loadAccounts();
    void saveAccounts();
    void loadAdmins();
    void saveAdmins();
    void loadTransactions();
    void saveTransactions();
    void loadAudit();
    void saveAudit();
};

#endif
