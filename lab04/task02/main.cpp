
#include "bankaccount.h"

using namespace std;

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

template<typename T>
T safeInput(const string& prompt) {
    T value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            clearInput();
            return value;
        } else {
            cout << "✗ Некоректне введення! Спробуйте ще раз.\n" << endl;
            clearInput();
        }
    }
}

void displayMainMenu() {
    cout << endl
         << "╭────────────────────────────────────────────╮" << endl
         << "║            ГОЛОВНЕ МЕНЮ БАНКУ              ║" << endl
         << "╰────────────────────────────────────────────╯" << endl
         << "1. Переглянути всі рахунки" << endl
         << "2. Переглянути інформацію про рахунок" << endl
         << "3. Поповнити рахунок" << endl
         << "4. Зняти кошти з рахунку" << endl
         << "5. Нарахувати відсотки (тільки для рахунків з відсотками)" << endl
         << "6. Створити новий рахунок" << endl
         << "7. Видалити рахунок" << endl
         << "0. Вийти з програми" << endl
         << "────────────────────────────────────────────" << endl;
}

void displayAccountsList(const vector<unique_ptr<BankAccount>>& accounts) {
    if (accounts.empty()) {
        cout << "\n✗ Немає доступних рахунків." << endl;
        return;
    }

    cout << endl
         << "╭────────────────────────────────────────────────────────────────╮" << endl
         << "║                      СПИСОК РАХУНКІВ                           ║" << endl
         << "╰────────────────────────────────────────────────────────────────╯" << endl;

    cout << left
         << setw(10) << "№"
         << setw(20) << "Номер"
         << setw(22) << "Власник"
         << setw(25) << "Баланс"
         << "Тип" << endl;

    cout << "────────────────────────────────────────────────────────────────" << endl;

    for (size_t i = 0; i < accounts.size(); i++) {
        cout << left
             << setw(5)  << (i + 1)
             << setw(15) << accounts[i]->getAccountNumber()
             << setw(30) << accounts[i]->getOwnerName()
             << setw(15) << fixed << setprecision(2) << accounts[i]->getBalance()
             << accounts[i]->getAccountType()
             << endl;
    }

    cout << "────────────────────────────────────────────────────────────────\n" << endl;
}

int selectAccount(const vector<unique_ptr<BankAccount>>& accounts) {
    if (accounts.empty()) {
        cout << "\n✗ Немає доступних рахунків." << endl;
        return -1;
    }

    displayAccountsList(accounts);
    int choice = safeInput<int>("Введіть номер рахунку (або 0 для повернення): ");

    if (choice == 0) return -1;
    if (choice < 1 || choice > static_cast<int>(accounts.size())) {
        cout << "✗ Некоректний номер рахунку!" << endl;
        return -1;
    }

    return choice - 1;
}

void viewAccountInfo(const vector<unique_ptr<BankAccount>>& accounts) {
    int index = selectAccount(accounts);
    if (index == -1) return;

    cout << "\n" << accounts[index]->getAccountInfo() << "\n" << endl;
}

void depositToAccount(vector<unique_ptr<BankAccount>>& accounts) {
    int index = selectAccount(accounts);
    if (index == -1) return;

    double amount = safeInput<double>("Введіть суму для поповнення: ");

    try {
        accounts[index]->deposit(amount);
        cout << "\n✓ Рахунок успішно поповнено!" << endl
             << "Новий баланс: " << fixed << setprecision(2)
             << accounts[index]->getBalance() << " грн\n" << endl;
    } catch (const exception& e) {
        cout << "\n✗ ПОМИЛКА: " << e.what() << "\n" << endl;
    }
}

void withdrawFromAccount(vector<unique_ptr<BankAccount>>& accounts) {
    int index = selectAccount(accounts);
    if (index == -1) return;

    double amount = safeInput<double>("Введіть суму для зняття: ");

    try {
        accounts[index]->withdraw(amount);
        cout << "\n✓ Кошти успішно знято!" << endl
             << "Новий баланс: " << fixed << setprecision(2)
             << accounts[index]->getBalance() << " грн\n" << endl;
    } catch (const exception& e) {
        cout << "\n✗ ПОМИЛКА: " << e.what() << "\n" << endl;
    }
}

void applyInterestToAccount(vector<unique_ptr<BankAccount>>& accounts) {
    int index = selectAccount(accounts);
    if (index == -1) return;

    InterestAccount* interestAcc = dynamic_cast<InterestAccount*>(accounts[index].get());

    if (!interestAcc) {
        cout << "\n✗ ПОМИЛКА: Цей рахунок не має відсоткової ставки!" << endl
             << "Нарахування відсотків доступне тільки для рахунків з відсотковою ставкою.\n" << endl;
        return;
    }

    double oldBalance = interestAcc->getBalance();
    interestAcc->applyInterest();
    double newBalance = interestAcc->getBalance();
    double interest = newBalance - oldBalance;

    cout << "\n✓ Відсотки успішно нараховано!" << endl
         << "Нараховано: " << fixed << setprecision(2) << interest << " грн" << endl
         << "Новий баланс: " << newBalance << " грн\n" << endl;
}

void createNewAccount(vector<unique_ptr<BankAccount>>& accounts) {
    cout << endl
         << "╭────────────────────────────────────────────╮" << endl
         << "║         СТВОРЕННЯ НОВОГО РАХУНКУ           ║" << endl
         << "╰────────────────────────────────────────────╯" << endl
         << "1. Звичайний рахунок\n"
         << "2. Рахунок з відсотковою ставкою\n"
         << "0. Повернутися\n"
         << "────────────────────────────────────────────" << endl;

    int type = safeInput<int>("Оберіть тип рахунку: ");
    if (type == 0) return;

    if (type != 1 && type != 2) {
        cout << "✗ Некоректний тип рахунку!" << endl;
        return;
    }

    clearInput();

    string accountNumber, ownerName;
    double balance;

    cout << "\nВведіть номер рахунку (мінімум 5 символів): ";
    getline(cin, accountNumber);

    cout << "Введіть ім'я власника: ";
    getline(cin, ownerName);

    balance = safeInput<double>("Введіть початковий баланс: ");

    try {
        if (type == 1) {
            double minBalance = safeInput<double>("Введіть мінімальний дозволений баланс: ");
            accounts.push_back(make_unique<RegularAccount>(accountNumber, ownerName, balance, minBalance));
            cout << "\n✓ Звичайний рахунок успішно створено!\n" << endl;
        } else {
            double interestRate = safeInput<double>("Введіть відсоткову ставку (%): ");
            accounts.push_back(make_unique<InterestAccount>(accountNumber, ownerName, balance, interestRate));
            cout << "\n✓ Рахунок з відсотковою ставкою успішно створено!\n" << endl;
        }
    } catch (const exception& e) {
        cout << "\n✗ ПОМИЛКА ПРИ СТВОРЕННІ РАХУНКУ: " << e.what() << "\n" << endl;
    }
}

void deleteAccount(vector<unique_ptr<BankAccount>>& accounts) {
    int index = selectAccount(accounts);
    if (index == -1) return;

    cout << "\n⚠ УВАГА! Ви впевнені, що хочете видалити рахунок?" << endl
         << "Номер: " << accounts[index]->getAccountNumber() << endl
         << "Власник: " << accounts[index]->getOwnerName() << endl
         << "Баланс: " << accounts[index]->getBalance() << " грн" << endl;

    clearInput();
    cout << "\nВведіть 'ТАК' для підтвердження: ";
    string confirmation;
    getline(cin, confirmation);

    if (confirmation == "ТАК" || confirmation == "так" || confirmation == "YES" || confirmation == "yes" || confirmation == "y" || confirmation == "Y") {
        accounts.erase(accounts.begin() + index);
        cout << "\n✓ Рахунок успішно видалено!\n" << endl;
    } else {
        cout << "\n✗ Видалення скасовано.\n" << endl;
    }
}

void initializeDefaultAccounts(vector<unique_ptr<BankAccount>>& accounts) {

    try {
        accounts.push_back(make_unique<RegularAccount>("UA12345001", "Курiщенко Павло", 10000.0, 1000.0));
        accounts.push_back(make_unique<RegularAccount>("UA12345002", "Петренко Марія", 15000.0, 2000.0));
        accounts.push_back(make_unique<InterestAccount>("UA12345003", "Сидоренко Олег", 50000.0, 5.5));
        accounts.push_back(make_unique<InterestAccount>("UA12345004", "Коваленко Анна", 100000.0, 7.0));
    } catch (const exception& e) {
        cout << "✗ ПОМИЛКА: " << e.what() << endl;
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    cout << "╭──────────────────────────────────────────────╮" << endl;
    cout << "║   СИСТЕМА КЕРУВАННЯ БАНКІВСЬКИМИ РАХУНКАМИ   ║" << endl;
    cout << "╰──────────────────────────────────────────────╯" << endl;

    cout << "\t\t\t⠀⠀⠀⠀⢀⠠⠤⠀⢀⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
         << "\t\t\t⠀⠀⠐⠀⠐⠀⠀⢀⣾⣿⡇⠀⠀⠀ ⠀⠀⢀⣼⡇⠀⠀⠀⠀\n"
         << "\t\t\t⠀⠀⠀⠀⠀⠀⠀⣸⣿⣿⡇⠀⠀⠀ ⠀⣴⣿⣿⠃⠀⠀⠀⠀\n"
         << "\t\t\t⠀⠀⠀⠀⠀⠀⢠⣿⣿⣿⣇⠀⠀⢀⣾⣿⣿⣿⠀⠀⠀⠀⠀\n"
         << "\t\t\t⠀⠀⠀⠀⠀⣴⣿⣿⣿⣿⣿⣿⣷⣿⣿⣿⣿⡟⠀⠀⠐⠀⡀\n"
         << "\t\t\t⠀⠀⠀⠀⢰⡿⠉⠀⡜⣿⣿⣿⡿⠿⢿⣿⣿⠃⠀⠀⠂⠄⠀\n"
         << "\t\t\t⠀⠀⠒⠒⠸⣿⣄⡘⣃⣿⣿⡟⢰⠃⠀⢹⣿⡇⠀⠀⠀⠀⠀\n"
         << "\t\t\t⠀⠀⠚⠉⠀⠈⠻⣿⣿⣿⣿⣿⣮⣤⣤⣿⡟⠁⠘⠠⠁⠀⠀\n"
         << "\t\t\t⠀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠛⠛⠛⠛⠛⠁⠀⠒⠤⠀⠀⠀⠀\n"
         << "\t\t\t⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠑⠀⠀⠀⠀⠀⠀\n";

    vector<unique_ptr<BankAccount>> accounts;

    initializeDefaultAccounts(accounts);

    bool running = true;
    while (running) {
        displayMainMenu();
        int choice = safeInput<int>("Оберіть опцію: ");

        switch (choice) {
        case 1:
            displayAccountsList(accounts);
            break;

        case 2:
            viewAccountInfo(accounts);
            break;

        case 3:
            depositToAccount(accounts);
            break;

        case 4:
            withdrawFromAccount(accounts);
            break;

        case 5:
            applyInterestToAccount(accounts);
            break;

        case 6:
            createNewAccount(accounts);
            break;

        case 7:
            deleteAccount(accounts);
            break;

        case 0:
            cout << endl
                 << "╭────────────────────────────────────────────╮" << endl
                 << "║       Дякуємо за використання системи!     ║" << endl
                 << "╰────────────────────────────────────────────╯" << endl;
            running = false;
            break;

        default:
            cout << "\n✗ Некоректний вибір! Спробуйте ще раз.\n" << endl;
            break;
        }
    }

    return 0;
}
