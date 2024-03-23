#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>

const int INITIAL_CAPACITY = 10;
const double LOAD_FACTOR_THRESHOLD = 0.5;

struct User {
    std::string username;
    std::string password;
    User* next;
};

class PasswordManager {
private:
    User** users;
    int capacity;
    int size;

    int hash(const std::string& key, int attempt) {
        int sum = 0;
        for (char c : key) {
            sum += c;
        }
        return (sum + attempt * secondaryHash(key)) % capacity;
    }

    int secondaryHash(const std::string& key) {
        int prime = 7;
        int hash = 0;
        for (char c : key) {
            hash = hash * prime + c;
        }
        return hash;
    }

    std::string encrypt(const std::string& password) {
        std::string encryptedPassword = password;
        for (char& c : encryptedPassword) {
            if (isalpha(c)) {
                char base = islower(c) ? 'a' : 'A';
                c = (c - base + 1) % 26 + base;
            }
        }
        return encryptedPassword;
    }

    std::string decrypt(const std::string& encryptedPassword) {
        std::string decryptedPassword = encryptedPassword;
        for (char& c : decryptedPassword) {
            if (isalpha(c)) {
                char base = islower(c) ? 'a' : 'A';
                c = (c - base - 1 + 26) % 26 + base;
            }
        }
        return decryptedPassword;
    }

    void rehash() {
        int newCapacity = capacity * 2;
        User** newUsers = new User*[newCapacity];
        for (int i = 0; i < newCapacity; ++i) {
            newUsers[i] = nullptr;
        }
        for (int i = 0; i < capacity; ++i) {
            User* curr = users[i];
            while (curr != nullptr) {
                User* temp = curr->next;
                int index = hash(curr->username, 0) % newCapacity;
                curr->next = newUsers[index];
                newUsers[index] = curr;
                curr = temp;
            }
        }
        capacity = newCapacity;
        delete[] users;
        users = newUsers;
    }

    double calculateLoadFactor() {
        return static_cast<double>(size) / capacity;
    }

public:
    PasswordManager() : capacity(INITIAL_CAPACITY), size(0) {
        users = new User*[capacity] { nullptr };
    }

    ~PasswordManager() {
        for (int i = 0; i < capacity; ++i) {
            User* curr = users[i];
            while (curr != nullptr) {
                User* temp = curr->next;
                delete curr;
                curr = temp;
            }
        }
        delete[] users;
    }

    void addPassword(const std::string& username, const std::string& password) {
        std::string encryptedPassword = encrypt(password);
        int index = hash(username, 0) % capacity;
        int attempt = 1;
        while (users[index] != nullptr) {
            index = (index + attempt * secondaryHash(username)) % capacity;
            attempt++;
        }
        users[index] = new User{username, encryptedPassword, nullptr};
        size++;
        if (calculateLoadFactor() > LOAD_FACTOR_THRESHOLD) {
            rehash();
        }
        std::ofstream userFile(username + ".txt", std::ios::app);
        userFile << encryptedPassword << std::endl;
        userFile.close();
    }

    void generateAndAddPassword(const std::string& username) {
        // Generate a random secure password
        const std::string symbols = "!@#$%^&*()_-+=<>?/[]{},.:;";
        const std::string numbers = "0123456789";
        const std::string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const std::string lowercase = "abcdefghijklmnopqrstuvwxyz";

        std::string randomPassword;
        srand(time(nullptr)); // Seed the random number generator

        // Add a random symbol
        randomPassword += symbols[rand() % symbols.size()];

        // Add a random number
        randomPassword += numbers[rand() % numbers.size()];

        // Add a random uppercase letter
        randomPassword += uppercase[rand() % uppercase.size()];

        // Fill the rest with lowercase letters
        for (int i = 0; i < 9; ++i) {
            randomPassword += lowercase[rand() % lowercase.size()];
        }

        addPassword(username, randomPassword);
    }

    void retrievePasswords(const std::string& username) {
        int index = hash(username, 0) % capacity;
        std::cout << "Saved passwords for " << username << ":\n";
        User* curr = users[index];
        while (curr != nullptr) {
            if (curr->username == username) {
                std::cout << decrypt(curr->password) << std::endl;
            }
            curr = curr->next;
        }
        std::ifstream userFile(username + ".txt");
        std::string storedPassword;
        while (std::getline(userFile, storedPassword)) {
            std::cout << decrypt(storedPassword) << std::endl;
        }
        userFile.close();
    }
    
    bool loginUser(const std::string& username, const std::string& password) {
        int index = hash(username, 0) % capacity;
        User* curr = users[index];
        while (curr != nullptr) {
            if (curr->username == username && encrypt(password) == curr->password) {
                return true;
            }
            curr = curr->next;
        }
        return false;
    }
};

int main() {
    PasswordManager manager;

    // Predefined user list
    const std::pair<std::string, std::string> userList[] = {
        {"Abdullah", "123"},
        {"Osama", "123"},
        {"Mahdy", "123"}
    };
    const int userListSize = sizeof(userList) / sizeof(userList[0]);

    // Login process
    std::string username, password;
    bool loggedIn = false;
    while (!loggedIn) {
        std::cout << "Enter username: ";
        std::cin >> username;
        std::cout << "Enter password: ";
        std::cin >> password;
        for (int i = 0; i < userListSize; ++i) {
            if (userList[i].first == username && userList[i].second == password) {
                loggedIn = true;
                std::cout << "Logged in successfully!\n";
                break;
            }
        }
        if (!loggedIn) {
            std::cout << "Invalid username or password. Please try again.\n";
        }
    }

    // User options
    std::string choice;
    while (true) {
        std::cout << "Choose an option:\n"
                     "1. Enter new password\n"
                     "2. Generate a password\n"
                     "3. Retrieve saved passwords\n"
                     "4. Logout\n"
                     "Enter your choice: ";
        std::cin >> choice;
        if (choice == "1") {
            std::string newPassword;
            std::cout << "Enter new password: ";
            std::cin >> newPassword;
            manager.addPassword(username, newPassword);
            std::cout << "Password added successfully!\n";
        } else if (choice == "2") {
            manager.generateAndAddPassword(username);
            std::cout << "Password generated and added successfully!\n";
        } else if (choice == "3") {
            manager.retrievePasswords(username);
        } else if (choice == "4") {
            std::cout << "Logged out successfully.\n";
            break;
        } else {
            std::cout << "Invalid choice. Please try again.\n";
        }
    }

    return 0;
}

