#include "auth.h"

#include "bank.h"
#include "utils.h"

std::string AuthService::hashPin(const std::string& pin) { return hashPinBasic(pin); }

bool AuthService::loginCustomer(Bank& bank, int accountId, const std::string& pin) {
    auto customer = bank.findCustomerByAccountId(accountId);
    if (!customer || customer->getPinHash() != hashPin(pin)) {
        return false;
    }
    role_ = Role::Customer;
    activeAccountId_ = accountId;
    activeAdmin_.clear();
    return true;
}

bool AuthService::loginAdmin(Bank& bank, const std::string& username, const std::string& pin) {
    auto admin = bank.findAdminByUsername(username);
    if (!admin || admin->getPinHash() != hashPin(pin)) {
        return false;
    }
    role_ = Role::Admin;
    activeAccountId_ = -1;
    activeAdmin_ = admin->getUsername();
    return true;
}

void AuthService::logout() {
    role_ = Role::None;
    activeAccountId_ = -1;
    activeAdmin_.clear();
}

AuthService::Role AuthService::getRole() const { return role_; }
int AuthService::getActiveAccountId() const { return activeAccountId_; }
const std::string& AuthService::getActiveAdmin() const { return activeAdmin_; }
