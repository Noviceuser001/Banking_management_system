# Banking_management_system

Console-based Bank Management System in modern C++ with:
- OOP design (`Person`, `Customer`, `Admin`, abstract `Account`, `SavingsAccount`, `CurrentAccount`, `Transaction`, `Bank`, `AuthService`)
- File persistence (`customers.dat`, `accounts.dat`, `transactions.dat`, `admins.dat`, `config.dat`, `audit.log`)
- PIN hashing (PINs are never stored in plain text)
- Customer and Admin workflows required in the project brief

## Build
```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o bank_app
```

## Run
```bash
./bank_app
```

Default admin credentials on first run:
- Username: `admin`
- PIN: `admin123`
