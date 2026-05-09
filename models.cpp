#include "models.h"

#include <utility>

Person::Person(int id, std::string name, std::string email, std::string phone)
    : id_(id), name_(std::move(name)), email_(std::move(email)), phone_(std::move(phone)) {}

int Person::getId() const { return id_; }
const std::string& Person::getName() const { return name_; }
const std::string& Person::getEmail() const { return email_; }
const std::string& Person::getPhone() const { return phone_; }

void Person::updateProfile(const std::string& name, const std::string& email, const std::string& phone) {
    name_ = name;
    email_ = email;
    phone_ = phone;
}

Customer::Customer(int id, std::string name, std::string email, std::string phone, int accountId, std::string pinHash,
                   bool closureRequested)
    : Person(id, std::move(name), std::move(email), std::move(phone)), accountId_(accountId),
      pinHash_(std::move(pinHash)), closureRequested_(closureRequested) {}

int Customer::getAccountId() const { return accountId_; }
const std::string& Customer::getPinHash() const { return pinHash_; }
bool Customer::isClosureRequested() const { return closureRequested_; }
void Customer::setClosureRequested(bool v) { closureRequested_ = v; }

Admin::Admin(int id, std::string name, std::string email, std::string phone, std::string username, std::string pinHash)
    : Person(id, std::move(name), std::move(email), std::move(phone)), username_(std::move(username)),
      pinHash_(std::move(pinHash)) {}

const std::string& Admin::getUsername() const { return username_; }
const std::string& Admin::getPinHash() const { return pinHash_; }

Account::Account(int accountId, int customerId, double balance, bool frozen)
    : accountId_(accountId), customerId_(customerId), balance_(balance), frozen_(frozen) {}

int Account::getAccountId() const { return accountId_; }
int Account::getCustomerId() const { return customerId_; }
double Account::getBalance() const { return balance_; }
void Account::setBalance(double balance) { balance_ = balance; }
bool Account::isFrozen() const { return frozen_; }
void Account::setFrozen(bool frozen) { frozen_ = frozen; }

bool Account::canWithdraw(double amount) const { return (balance_ - amount) >= -getOverdraftLimit(); }

std::string SavingsAccount::getType() const { return "Savings"; }
double SavingsAccount::getOverdraftLimit() const { return 0.0; }

double SavingsAccount::applyInterest(double annualRate) {
    if (annualRate <= 0.0 || balance_ <= 0.0) {
        return 0.0;
    }
    const double interest = balance_ * annualRate;
    balance_ += interest;
    return interest;
}

std::string CurrentAccount::getType() const { return "Current"; }
double CurrentAccount::getOverdraftLimit() const { return kDefaultOverdraft; }

double CurrentAccount::applyInterest(double annualRate) {
    if (annualRate <= 0.0 || balance_ <= 0.0) {
        return 0.0;
    }
    const double interest = balance_ * annualRate;
    balance_ += interest;
    return interest;
}

Transaction::Transaction(int txId, std::string timestamp, std::string type, int fromAccount, int toAccount,
                         double amount, std::string note)
    : txId_(txId), timestamp_(std::move(timestamp)), type_(std::move(type)), fromAccount_(fromAccount),
      toAccount_(toAccount), amount_(amount), note_(std::move(note)) {}

int Transaction::getTxId() const { return txId_; }
const std::string& Transaction::getTimestamp() const { return timestamp_; }
const std::string& Transaction::getType() const { return type_; }
int Transaction::getFromAccount() const { return fromAccount_; }
int Transaction::getToAccount() const { return toAccount_; }
double Transaction::getAmount() const { return amount_; }
const std::string& Transaction::getNote() const { return note_; }
