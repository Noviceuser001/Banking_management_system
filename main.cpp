#include <iostream>

#include "auth.h"
#include "bank.h"
#include "ui.h"
#include "utils.h"

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
