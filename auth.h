#ifndef AUTH_H
#define AUTH_H

#include <string>

class Bank;

class AuthService {
  public:
    enum class Role { None, Customer, Admin };

  private:
    Role role_ = Role::None;
    int activeAccountId_ = -1;
    std::string activeAdmin_;

  public:
    static std::string hashPin(const std::string& pin);

    bool loginCustomer(Bank& bank, int accountId, const std::string& pin);
    bool loginAdmin(Bank& bank, const std::string& username, const std::string& pin);
    void logout();

    Role getRole() const;
    int getActiveAccountId() const;
    const std::string& getActiveAdmin() const;
};

#endif
