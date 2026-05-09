# Code Structure

## Overview
The program is split into focused headers and source files. Each header declares the public types or functions, and each .cpp file contains the implementations.

## Files
- main.cpp
  - Entry point and main loop.
- utils.h / utils.cpp
  - Helper utilities for input, string handling, timestamps, hashing, and constants.
- models.h / models.cpp
  - Core data models: Person, Customer, Admin, Account, SavingsAccount, CurrentAccount, Transaction.
- bank.h / bank.cpp
  - Bank class that manages persistence, customers, accounts, transactions, and auditing.
- auth.h / auth.cpp
  - Authentication service for customers and admins.
- ui.h / ui.cpp
  - Menu rendering and user interaction for customer and admin flows.

## Build
Compile all .cpp files together. Example command:

```
g++ -std=c++17 -O2 -o banking_system *.cpp
```
