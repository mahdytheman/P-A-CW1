#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>

const int INITIAL_CAPACITY = 10;
const double LOAD_FACTOR_THRESHOLD = 0.5;

// Structure to represent a user with username, password, and pointer to next user
struct User {
    std::string username;
    std::string password;
    User* next;
};

// Class to manage passwords
class PasswordManager {
private:
    // Array of pointers to users, capacity, and size of the array
    User** users;
    int capacity;
    int size;

    // Hash function to map a string key to an index
    int hash(const std::string& key, int attempt) {
        int sum = 0;
        for (char c : key) {
            sum += c;
        }
        return (sum + attempt * secondaryHash(key)) % capacity;
    }

    // Secondary hash function used within the primary hash function
    int secondaryHash(const std::string& key) {
        int prime = 7;
        int hash = 0;
        for (char c : key) {
            hash = hash * prime + c;
        }
        return hash;
    }

    // Function to encrypt a password
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

    // Function to decrypt an encrypted password
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

    // Function to rehash the users array when load factor exceeds threshold
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

    // Function to calculate load factor of the hash table
    double calculateLoadFactor() {
        return static_cast<double>(size) / capacity;
    }

public:
    // Constructor to initialize capacity and size, and create users array
    PasswordManager() : capacity(INITIAL_CAPACITY), size(0) {
        users = new User*[capacity] { nullptr };
    }

    // Destructor to deallocate memory used by users array
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

    // Function to add a password for a given username
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

    // Function to generate a random password and add it for a given username
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

    // Function to retrieve saved passwords for a given username
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
    
    // Function to verify login credentials
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

    // Function to delete a password for a given username and password
    void deletePassword(const std::string& username, const std::string& passwordToDelete) {
        int index = hash(username, 0) % capacity;
        User* prev = nullptr;
        User* curr = users[index];
        while (curr != nullptr) {
            if (curr->username == username && decrypt(curr->password) == passwordToDelete) {
                if (prev != nullptr) {
                    prev->next = curr->next;
                } else {
                    users[index] = curr->next;
                }
                delete curr;
                size--;
                return;
            }
            prev = curr;
            curr = curr->next;
        }
        std::cout << "Password not found for deletion.\n";
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
                    "4. Delete password\n" // Added option for deleting password
                    "5. Logout\n"
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
        } else if (choice == "4") { // Option for deleting password
            std::string passwordToDelete;
            std::cout << "Enter password to delete: ";
            std::cin >> passwordToDelete;
            manager.deletePassword(username, passwordToDelete);
        } else if (choice == "5") {
            std::cout << "Logged out successfully.\n";
            break;
        } else {
            std::cout << "Invalid choice. Please try again.\n";
        }
    }

    return 0;
}
