#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr std::size_t MINI_STATEMENT_SIZE = 5;

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

std::string sanitizeField(const std::string& input) {
    std::string out = input;
    std::replace(out.begin(), out.end(), '|', '/');
    return out;
}

std::vector<std::string> split(const std::string& line, char delim = '|') {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, delim)) {
        parts.push_back(item);
    }
    return parts;
}

std::string nowTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return os.str();
}

std::string hashPinBasic(const std::string& pin) {
    unsigned long long h = 1469598103934665603ULL;
    for (const unsigned char c : pin) {
        h ^= static_cast<unsigned long long>(c);
        h *= 1099511628211ULL;
        h ^= (h >> 13);
    }
    std::ostringstream os;
    os << std::hex << h;
    return os.str();
}

std::string readLine(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

int readInt(const std::string& prompt) {
    while (true) {
        try {
            const std::string s = readLine(prompt);
            size_t pos = 0;
            int value = std::stoi(s, &pos);
            if (pos == s.size()) {
                return value;
            }
        } catch (...) {
        }
        std::cout << "Invalid number. Try again.\n";
    }
}

double readDouble(const std::string& prompt) {
    while (true) {
        try {
            const std::string s = readLine(prompt);
            size_t pos = 0;
            double value = std::stod(s, &pos);
            if (pos == s.size()) {
                return value;
            }
        } catch (...) {
        }
        std::cout << "Invalid amount. Try again.\n";
    }
}

} // namespace

class Person {
  protected:
    int id_{};
    std::string name_;
    std::string email_;
    std::string phone_;

  public:
    Person() = default;
    Person(int id, std::string name, std::string email, std::string phone)
        : id_(id), name_(std::move(name)), email_(std::move(email)), phone_(std::move(phone)) {}

    virtual ~Person() = default;

    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getPhone() const { return phone_; }

    void updateProfile(const std::string& name, const std::string& email, const std::string& phone) {
        name_ = name;
        email_ = email;
        phone_ = phone;
    }
};

class Customer : public Person {
  private:
    int accountId_{};
    std::string pinHash_;
    bool closureRequested_ = false;

  public:
    Customer() = default;
    Customer(int id, std::string name, std::string email, std::string phone, int accountId, std::string pinHash,
             bool closureRequested = false)
        : Person(id, std::move(name), std::move(email), std::move(phone)), accountId_(accountId),
          pinHash_(std::move(pinHash)), closureRequested_(closureRequested) {}

    int getAccountId() const { return accountId_; }
    const std::string& getPinHash() const { return pinHash_; }
    bool isClosureRequested() const { return closureRequested_; }
    void setClosureRequested(bool v) { closureRequested_ = v; }
};

class Admin : public Person {
  private:
    std::string username_;
    std::string pinHash_;

  public:
    Admin() = default;
    Admin(int id, std::string name, std::string email, std::string phone, std::string username, std::string pinHash)
        : Person(id, std::move(name), std::move(email), std::move(phone)), username_(std::move(username)),
          pinHash_(std::move(pinHash)) {}

    const std::string& getUsername() const { return username_; }
    const std::string& getPinHash() const { return pinHash_; }
};

class Account {
  protected:
    int accountId_{};
    int customerId_{};
    double balance_{};
    bool frozen_ = false;

  public:
    Account() = default;
    Account(int accountId, int customerId, double balance, bool frozen)
        : accountId_(accountId), customerId_(customerId), balance_(balance), frozen_(frozen) {}

    virtual ~Account() = default;

    int getAccountId() const { return accountId_; }
    int getCustomerId() const { return customerId_; }
    double getBalance() const { return balance_; }
    void setBalance(double balance) { balance_ = balance; }
    bool isFrozen() const { return frozen_; }
    void setFrozen(bool frozen) { frozen_ = frozen; }

    virtual std::string getType() const = 0;
    virtual double getOverdraftLimit() const = 0;
    virtual double applyInterest(double annualRate) = 0;

    bool canWithdraw(double amount) const { return (balance_ - amount) >= -getOverdraftLimit(); }
};

class SavingsAccount : public Account {
  public:
    using Account::Account;

    std::string getType() const override { return "Savings"; }
    double getOverdraftLimit() const override { return 0.0; }

    double applyInterest(double annualRate) override {
        if (annualRate <= 0.0 || balance_ <= 0.0) {
            return 0.0;
        }
        const double interest = balance_ * annualRate;
        balance_ += interest;
        return interest;
    }
};

class CurrentAccount : public Account {
  public:
    static constexpr double kDefaultOverdraft = 500.0;

    using Account::Account;

    std::string getType() const override { return "Current"; }
    double getOverdraftLimit() const override { return kDefaultOverdraft; }

    double applyInterest(double annualRate) override {
        if (annualRate <= 0.0 || balance_ <= 0.0) {
            return 0.0;
        }
        const double interest = balance_ * annualRate;
        balance_ += interest;
        return interest;
    }
};

class Transaction {
  private:
    int txId_{};
    std::string timestamp_;
    std::string type_;
    int fromAccount_{};
    int toAccount_{};
    double amount_{};
    std::string note_;

  public:
    Transaction() = default;
    Transaction(int txId, std::string timestamp, std::string type, int fromAccount, int toAccount, double amount,
                std::string note)
        : txId_(txId), timestamp_(std::move(timestamp)), type_(std::move(type)), fromAccount_(fromAccount),
          toAccount_(toAccount), amount_(amount), note_(std::move(note)) {}

    int getTxId() const { return txId_; }
    const std::string& getTimestamp() const { return timestamp_; }
    const std::string& getType() const { return type_; }
    int getFromAccount() const { return fromAccount_; }
    int getToAccount() const { return toAccount_; }
    double getAmount() const { return amount_; }
    const std::string& getNote() const { return note_; }
};

class AuthService;

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
    Bank() { loadAll(); }

    void addAudit(const std::string& message) { auditLog_.push_back(nowTimestamp() + " | " + message); }

    const std::vector<std::string>& getAuditLog() const { return auditLog_; }
    const std::vector<std::shared_ptr<Account>>& getAllAccounts() const { return accounts_; }
    const std::vector<std::shared_ptr<Customer>>& getAllCustomers() const { return customers_; }
    const std::vector<std::shared_ptr<Admin>>& getAllAdmins() const { return admins_; }

    std::shared_ptr<Account> findAccountById(int accountId) {
        for (const auto& account : accounts_) {
            if (account->getAccountId() == accountId) {
                return account;
            }
        }
        return nullptr;
    }

    std::shared_ptr<Customer> findCustomerById(int customerId) {
        for (const auto& customer : customers_) {
            if (customer->getId() == customerId) {
                return customer;
            }
        }
        return nullptr;
    }

    std::shared_ptr<Customer> findCustomerByAccountId(int accountId) {
        for (const auto& customer : customers_) {
            if (customer->getAccountId() == accountId) {
                return customer;
            }
        }
        return nullptr;
    }

    std::shared_ptr<Admin> findAdminByUsername(const std::string& username) {
        const std::string key = toLower(username);
        for (const auto& admin : admins_) {
            if (toLower(admin->getUsername()) == key) {
                return admin;
            }
        }
        return nullptr;
    }

    std::optional<int> registerCustomer(const std::string& name, const std::string& email, const std::string& phone,
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

        auto customer = std::make_shared<Customer>(customerId, name, email, phone, accountId, hashPinBasic(pin), false);

        accounts_.push_back(account);
        customers_.push_back(customer);

        if (initialDeposit > 0.0) {
            deposit(accountId, initialDeposit, "Initial deposit");
        }

        addAudit("Registered customer " + sanitizeField(name) + " with account " + std::to_string(accountId));
        return accountId;
    }

    bool deposit(int accountId, double amount, const std::string& note = "Deposit", bool createTx = true) {
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

    bool withdraw(int accountId, double amount, const std::string& note = "Withdrawal", bool createTx = true) {
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

    bool transfer(int fromAccount, int toAccount, double amount) {
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

    bool updateProfile(int accountId, const std::string& name, const std::string& email, const std::string& phone) {
        auto customer = findCustomerByAccountId(accountId);
        if (!customer) {
            return false;
        }
        customer->updateProfile(name, email, phone);
        addAudit("Profile updated for account " + std::to_string(accountId));
        return true;
    }

    bool requestClosure(int accountId) {
        auto customer = findCustomerByAccountId(accountId);
        if (!customer) {
            return false;
        }
        customer->setClosureRequested(true);
        addAudit("Closure requested for account " + std::to_string(accountId));
        return true;
    }

    bool applyInterest(int accountId) {
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

    std::vector<Transaction> getTransactionsForAccount(int accountId) const {
        std::vector<Transaction> result;
        for (const auto& tx : transactions_) {
            if (tx.getFromAccount() == accountId || tx.getToAccount() == accountId) {
                result.push_back(tx);
            }
        }
        return result;
    }

    std::vector<std::shared_ptr<Account>> searchAccounts(const std::string& query) {
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

    bool freezeAccount(int accountId, bool frozen) {
        auto account = findAccountById(accountId);
        if (!account) {
            return false;
        }
        account->setFrozen(frozen);
        addAudit(std::string(frozen ? "Frozen " : "Unfrozen ") + "account " + std::to_string(accountId));
        return true;
    }

    bool deleteAccount(int accountId) {
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

    double getSavingsInterestRate() const { return savingsInterestRate_; }
    double getCurrentInterestRate() const { return currentInterestRate_; }

    bool setInterestRates(double savingsRate, double currentRate) {
        if (savingsRate < 0.0 || currentRate < 0.0) {
            return false;
        }
        savingsInterestRate_ = savingsRate;
        currentInterestRate_ = currentRate;
        addAudit("Updated interest rates. Savings=" + std::to_string(savingsRate) + " Current=" +
                 std::to_string(currentRate));
        return true;
    }

    std::vector<std::shared_ptr<Customer>> getClosureRequests() const {
        std::vector<std::shared_ptr<Customer>> requests;
        for (const auto& customer : customers_) {
            if (customer->isClosureRequested()) {
                requests.push_back(customer);
            }
        }
        return requests;
    }

    void generateReport() {
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

    void loadAll() {
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

    void saveAll() {
        saveConfig();
        saveAccounts();
        saveCustomers();
        saveAdmins();
        saveTransactions();
        saveAudit();
    }

  private:
    void loadConfig() {
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

    void saveConfig() {
        std::ofstream out(configFile_, std::ios::trunc);
        out << nextCustomerId_ << '|' << nextAccountId_ << '|' << nextTransactionId_ << '|' << nextAdminId_ << '|'
            << savingsInterestRate_ << '|' << currentInterestRate_ << '\n';
    }

    void loadCustomers() {
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
                customers_.push_back(std::make_shared<Customer>(std::stoi(p[0]), p[1], p[2], p[3], std::stoi(p[4]),
                                                                p[5], p[6] == "1"));
            } catch (...) {
                addAudit("Skipped malformed customer record values at line " + std::to_string(lineNo));
            }
        }
    }

    void saveCustomers() {
        std::ofstream out(customersFile_, std::ios::trunc);
        for (const auto& c : customers_) {
            out << c->getId() << '|' << sanitizeField(c->getName()) << '|' << sanitizeField(c->getEmail()) << '|'
                << sanitizeField(c->getPhone()) << '|' << c->getAccountId() << '|' << c->getPinHash() << '|'
                << (c->isClosureRequested() ? 1 : 0) << '\n';
        }
    }

    void loadAccounts() {
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

    void saveAccounts() {
        std::ofstream out(accountsFile_, std::ios::trunc);
        for (const auto& a : accounts_) {
            out << a->getType() << '|' << a->getAccountId() << '|' << a->getCustomerId() << '|' << a->getBalance()
                << '|' << (a->isFrozen() ? 1 : 0) << '\n';
        }
    }

    void loadAdmins() {
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

    void saveAdmins() {
        std::ofstream out(adminsFile_, std::ios::trunc);
        for (const auto& a : admins_) {
            out << a->getId() << '|' << sanitizeField(a->getName()) << '|' << sanitizeField(a->getEmail()) << '|'
                << sanitizeField(a->getPhone()) << '|' << sanitizeField(a->getUsername()) << '|' << a->getPinHash()
                << '\n';
        }
    }

    void loadTransactions() {
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

    void saveTransactions() {
        std::ofstream out(transactionsFile_, std::ios::trunc);
        for (const auto& t : transactions_) {
            out << t.getTxId() << '|' << t.getTimestamp() << '|' << t.getType() << '|' << t.getFromAccount() << '|'
                << t.getToAccount() << '|' << t.getAmount() << '|' << sanitizeField(t.getNote()) << '\n';
        }
    }

    void loadAudit() {
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

    void saveAudit() {
        std::ofstream out(auditFile_, std::ios::trunc);
        for (const auto& line : auditLog_) {
            out << line << '\n';
        }
    }
};

class AuthService {
  public:
    enum class Role { None, Customer, Admin };

  private:
    Role role_ = Role::None;
    int activeAccountId_ = -1;
    std::string activeAdmin_;

  public:
    static std::string hashPin(const std::string& pin) { return hashPinBasic(pin); }

    bool loginCustomer(Bank& bank, int accountId, const std::string& pin) {
        auto customer = bank.findCustomerByAccountId(accountId);
        if (!customer || customer->getPinHash() != hashPin(pin)) {
            return false;
        }
        role_ = Role::Customer;
        activeAccountId_ = accountId;
        activeAdmin_.clear();
        return true;
    }

    bool loginAdmin(Bank& bank, const std::string& username, const std::string& pin) {
        auto admin = bank.findAdminByUsername(username);
        if (!admin || admin->getPinHash() != hashPin(pin)) {
            return false;
        }
        role_ = Role::Admin;
        activeAccountId_ = -1;
        activeAdmin_ = admin->getUsername();
        return true;
    }

    void logout() {
        role_ = Role::None;
        activeAccountId_ = -1;
        activeAdmin_.clear();
    }

    Role getRole() const { return role_; }
    int getActiveAccountId() const { return activeAccountId_; }
    const std::string& getActiveAdmin() const { return activeAdmin_; }
};

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

int main() {
    std::cout << "\n==== Welcome to Bank Management System ====\n";
    std::cout << "Data persistence is enabled (customers.dat, accounts.dat, transactions.dat, admins.dat).\n";

    Bank bank;
    AuthService auth;

    while (true) {
        std::cout << "\n--- Main Menu ---\n"
                  << "1. Customer Account Registration\n"
                  << "2. Customer Login\n"
                  << "3. Admin Login\n"
                  << "4. Exit\n";

        const int choice = readInt("Select option: ");

        switch (choice) {
        case 1: {
            const std::string name = readLine("Enter full name: ");
            const std::string email = readLine("Enter email: ");
            const std::string phone = readLine("Enter phone: ");
            const std::string pin = readLine("Set 4+ digit PIN: ");
            const std::string accType = readLine("Select account type (Savings/Current): ");
            const double initial = readDouble("Initial deposit (>= 0): ");

            auto accountId = bank.registerCustomer(name, email, phone, pin, accType, initial);
            if (accountId.has_value()) {
                std::cout << "Registration successful. Your Account ID is: " << *accountId << '\n';
            } else {
                std::cout << "Registration failed. Please check your inputs.\n";
            }
            break;
        }
        case 2: {
            const int accountId = readInt("Enter account ID: ");
            const std::string pin = readLine("Enter PIN: ");
            if (auth.loginCustomer(bank, accountId, pin)) {
                std::cout << "Customer login successful.\n";
                customerMenu(bank, auth);
            } else {
                std::cout << "Invalid account ID/PIN.\n";
            }
            break;
        }
        case 3: {
            const std::string username = readLine("Enter admin username: ");
            const std::string pin = readLine("Enter admin PIN: ");
            if (auth.loginAdmin(bank, username, pin)) {
                std::cout << "Admin login successful.\n";
                adminMenu(bank, auth);
            } else {
                std::cout << "Invalid admin credentials.\n";
            }
            break;
        }
        case 4:
            bank.saveAll();
            std::cout << "Data saved. Exiting...\n";
            return 0;
        default:
            std::cout << "Invalid option. Please try again.\n";
            break;
        }
    }
}
