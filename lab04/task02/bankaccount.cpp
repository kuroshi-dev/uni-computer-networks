#include "bankaccount.h"

// ==================== Realisation of class exception ====================

InvalidAccountNumberException::InvalidAccountNumberException(const std::string& accNum) {
    message = "Некоректний номер рахунку: " + accNum;
}

const char* InvalidAccountNumberException::what() const noexcept {
    return message.c_str();
}

NegativeBalanceException::NegativeBalanceException(double amount) {
    std::ostringstream oss;
    oss << "Некоректна сума балансу: " << amount << " (не може бути від'ємною)";
    message = oss.str();
}

const char* NegativeBalanceException::what() const noexcept {
    return message.c_str();
}

NegativeInterestRateException::NegativeInterestRateException(double rate) {
    std::ostringstream oss;
    oss << "Некоректна відсоткова ставка: " << rate << "% (не може бути від'ємною)";
    message = oss.str();
}

const char* NegativeInterestRateException::what() const noexcept {
    return message.c_str();
}

InsufficientFundsException::InsufficientFundsException(double balance, double amount) {
    std::ostringstream oss;
    oss << "Недостатньо коштів. Баланс: " << balance << ", потрібно: " << amount;
    message = oss.str();
}

const char* InsufficientFundsException::what() const noexcept {
    return message.c_str();
}

BelowMinimumBalanceException::BelowMinimumBalanceException(double balance, double minBalance) {
    std::ostringstream oss;
    oss << "Баланс (" << balance << ") нижче мінімального дозволеного (" << minBalance << ")";
    message = oss.str();
}

const char* BelowMinimumBalanceException::what() const noexcept {
    return message.c_str();
}

// ----------------- Realisation of BankAccount -----------------

BankAccount::BankAccount(const std::string& accNum, const std::string& owner, double bal)
    : accountNumber(accNum), ownerName(owner), balance(bal) {

    if (accNum.empty() || accNum.length() < 5) {
        throw InvalidAccountNumberException(accNum);
    }
    if (bal < 0) {
        throw NegativeBalanceException(bal);
    }
}

BankAccount::~BankAccount() {}

void BankAccount::deposit(double amount) {
    if (amount <= 0) {
        throw std::invalid_argument("Сума для поповнення має бути додатною");
    }
    balance += amount;
}

std::string BankAccount::getAccountNumber() const {
    return accountNumber;
}

std::string BankAccount::getOwnerName() const {
    return ownerName;
}

double BankAccount::getBalance() const {
    return balance;
}

void BankAccount::setOwnerName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Ім'я власника не може бути порожнім");
    }
    ownerName = name;
}

// ----------------- Realisation of RegularAccount -----------------

RegularAccount::RegularAccount(const std::string& accNum, const std::string& owner,
                               double bal, double minBal)
    : BankAccount(accNum, owner, bal), minimumBalance(minBal) {

    if (minBal < 0) {
        throw std::invalid_argument("Мінімальний баланс не може бути від'ємним");
    }
    if (bal < minBal) {
        throw BelowMinimumBalanceException(bal, minBal);
    }
}

void RegularAccount::withdraw(double amount) {
    if (amount <= 0) {
        throw std::invalid_argument("Сума для зняття має бути додатною");
    }
    if (amount > balance) {
        throw InsufficientFundsException(balance, amount);
    }
    if (balance - amount < minimumBalance) {
        throw BelowMinimumBalanceException(balance - amount, minimumBalance);
    }
    balance -= amount;
}

std::string RegularAccount::getAccountInfo() const {
    std::ostringstream oss;
    oss << "=== ЗВИЧАЙНИЙ РАХУНОК ===\n"
        << "Номер рахунку: " << accountNumber << "\n"
        << "Власник: " << ownerName << "\n"
        << "Баланс: " << balance << " грн\n"
        << "Мінімальний баланс: " << minimumBalance << " грн";
    return oss.str();
}

std::string RegularAccount::getAccountType() const {
    return "regular";
}

double RegularAccount::getMinimumBalance() const {
    return minimumBalance;
}

void RegularAccount::setMinimumBalance(double minBal) {
    if (minBal < 0) {
        throw std::invalid_argument("Мінімальний баланс не може бути від'ємним");
    }
    if (balance < minBal) {
        throw BelowMinimumBalanceException(balance, minBal);
    }
    minimumBalance = minBal;
}

// ----------------- Realisation of InterestAccount -----------------

InterestAccount::InterestAccount(const std::string& accNum, const std::string& owner,
                                 double bal, double rate)
    : BankAccount(accNum, owner, bal), interestRate(rate) {

    if (rate < 0) {
        throw NegativeInterestRateException(rate);
    }
}

void InterestAccount::withdraw(double amount) {
    if (amount <= 0) {
        throw std::invalid_argument("Сума для зняття має бути додатною");
    }
    if (amount > balance) {
        throw InsufficientFundsException(balance, amount);
    }
    balance -= amount;
}

void InterestAccount::applyInterest() {
    double interest = balance * (interestRate / 100.0);
    balance += interest;
}

double InterestAccount::calculateFutureBalance(int months) const {
    if (months < 0) {
        throw std::invalid_argument("Кількість місяців не може бути від'ємною");
    }
    double futureBalance = balance;
    for (int i = 0; i < months; i++) {
        futureBalance += futureBalance * (interestRate / 100.0);
    }
    return futureBalance;
}

std::string InterestAccount::getAccountInfo() const {
    std::ostringstream oss;
    oss << "=== РАХУНОК З ВІДСОТКАМИ ===\n"
        << "Номер рахунку: " << accountNumber << "\n"
        << "Власник: " << ownerName << "\n"
        << "Баланс: " << balance << " грн\n"
        << "Відсоткова ставка: " << interestRate << "%\n"
        << "Прогноз через 12 міс: " << calculateFutureBalance(12) << " грн";
    return oss.str();
}

std::string InterestAccount::getAccountType() const {
    return "interest";
}

double InterestAccount::getInterestRate() const {
    return interestRate;
}

void InterestAccount::setInterestRate(double rate) {
    if (rate < 0) {
        throw NegativeInterestRateException(rate);
    }
    interestRate = rate;
}
