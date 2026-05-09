#ifndef MODELS_H
#define MODELS_H

#include <memory>
#include <string>

class Person {
  protected:
    int id_{};
    std::string name_;
    std::string email_;
    std::string phone_;

  public:
    Person() = default;
    Person(int id, std::string name, std::string email, std::string phone);
    virtual ~Person() = default;

    int getId() const;
    const std::string& getName() const;
    const std::string& getEmail() const;
    const std::string& getPhone() const;

    void updateProfile(const std::string& name, const std::string& email, const std::string& phone);
};

class Customer : public Person {
  private:
    int accountId_{};
    std::string pinHash_;
    bool closureRequested_ = false;

  public:
    Customer() = default;
    Customer(int id, std::string name, std::string email, std::string phone, int accountId, std::string pinHash,
             bool closureRequested = false);

    int getAccountId() const;
    const std::string& getPinHash() const;
    bool isClosureRequested() const;
    void setClosureRequested(bool v);
};

class Admin : public Person {
  private:
    std::string username_;
    std::string pinHash_;

  public:
    Admin() = default;
    Admin(int id, std::string name, std::string email, std::string phone, std::string username, std::string pinHash);

    const std::string& getUsername() const;
    const std::string& getPinHash() const;
};

class Account {
  protected:
    int accountId_{};
    int customerId_{};
    double balance_{};
    bool frozen_ = false;

  public:
    Account() = default;
    Account(int accountId, int customerId, double balance, bool frozen);
    virtual ~Account() = default;

    int getAccountId() const;
    int getCustomerId() const;
    double getBalance() const;
    void setBalance(double balance);
    bool isFrozen() const;
    void setFrozen(bool frozen);

    virtual std::string getType() const = 0;
    virtual double getOverdraftLimit() const = 0;
    virtual double applyInterest(double annualRate) = 0;

    bool canWithdraw(double amount) const;
};

class SavingsAccount : public Account {
  public:
    using Account::Account;

    std::string getType() const override;
    double getOverdraftLimit() const override;
    double applyInterest(double annualRate) override;
};

class CurrentAccount : public Account {
  public:
    static constexpr double kDefaultOverdraft = 500.0;

    using Account::Account;

    std::string getType() const override;
    double getOverdraftLimit() const override;
    double applyInterest(double annualRate) override;
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
                std::string note);

    int getTxId() const;
    const std::string& getTimestamp() const;
    const std::string& getType() const;
    int getFromAccount() const;
    int getToAccount() const;
    double getAmount() const;
    const std::string& getNote() const;
};

#endif
