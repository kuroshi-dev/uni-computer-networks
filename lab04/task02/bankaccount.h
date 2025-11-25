#ifndef BANKACCOUNT_H
#define BANKACCOUNT_H

#include <iostream>
#include <vector>
#include <memory>
#include <limits>
#include <iomanip>
#include <string>
#include <windows.h>
#include <stdexcept>
#include <sstream>

class InvalidAccountNumberException : public std::exception {
private:
    std::string message;
public:
    InvalidAccountNumberException(const std::string& accNum);
    const char* what() const noexcept override;
};

class NegativeBalanceException : public std::exception {
private:
    std::string message;
public:
    NegativeBalanceException(double amount);
    const char* what() const noexcept override;
};

class NegativeInterestRateException : public std::exception {
private:
    std::string message;
public:
    NegativeInterestRateException(double rate);
    const char* what() const noexcept override;
};

class InsufficientFundsException : public std::exception {
private:
    std::string message;
public:
    InsufficientFundsException(double balance, double amount);
    const char* what() const noexcept override;
};

class BelowMinimumBalanceException : public std::exception {
private:
    std::string message;
public:
    BelowMinimumBalanceException(double balance, double minBalance);
    const char* what() const noexcept override;
};

class BankAccount {
protected:
    std::string accountNumber;
    std::string ownerName;
    double balance;

public:
    BankAccount(const std::string& accNum, const std::string& owner, double bal);

    virtual ~BankAccount();

    virtual void deposit(double amount);
    virtual void withdraw(double amount) = 0;
    virtual std::string getAccountInfo() const = 0;
    virtual std::string getAccountType() const = 0;

    std::string getAccountNumber() const;
    std::string getOwnerName() const;
    double getBalance() const;

    void setOwnerName(const std::string& name);
};

class RegularAccount : public BankAccount {
private:
    double minimumBalance;

public:
    RegularAccount(const std::string& accNum, const std::string& owner,
                   double bal, double minBal);

    void withdraw(double amount) override;
    std::string getAccountInfo() const override;
    std::string getAccountType() const override;

    double getMinimumBalance() const;
    void setMinimumBalance(double minBal);
};

class InterestAccount : public BankAccount {
private:
    double interestRate;

public:
    InterestAccount(const std::string& accNum, const std::string& owner,
                    double bal, double rate);

    void withdraw(double amount) override;

    void applyInterest();

    double calculateFutureBalance(int months) const;

    std::string getAccountInfo() const override;
    std::string getAccountType() const override;

    double getInterestRate() const;
    void setInterestRate(double rate);
};

#endif // BANKACCOUNT_H
