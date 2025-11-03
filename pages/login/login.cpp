#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// 用来declare有这个function
#include "../../struct/add_book.h"
#include "../../struct/book.h"
#include "../../struct/borrow_record.h"
#include "../../struct/borrower.h"
#include "../../struct/return_book.h"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

struct User {
   int id;
   string username;
   string password;
   int attempts;
   bool locked;
   string type;
   string created_at;
};

const string USER_FILE = "data/users.txt";

// global data
vector<Borrower> borrowers;
vector<Book> books;
vector<BorrowRecord> borrow_records;

const int WIDTH = 46;

vector<string> split(const string& line, char delimiter = '|') {
   vector<string> parts;
   stringstream ss(line);
   string part;
   while (getline(ss, part, delimiter)) parts.push_back(part);
   return parts;
}

vector<Borrower> loadBorrowers() {
   vector<Borrower> list;
   ifstream file("data/borrowers.txt");
   string line;

   while (getline(file, line)) {
      vector<string> d = split(line);
      list.push_back({stoi(d[0]), d[1], d[2], d[3], d[4], d[5]});
   }

   return list;
}

vector<Book> loadBooks() {
   books.clear();
   ifstream file("data/books.txt");
   string line;

   while (getline(file, line)) {
      vector<string> d = split(line);
      books.push_back({
          stoi(d[0]),  // id
          d[1],        // title
          d[2],        // author
          d[3],        // isbn
          stoi(d[4]),  // copies
          d[5],        // created_at
      });
   }

   return books;
}

vector<BorrowRecord> loadBorrowRecords() {
   borrow_records.clear();
   ifstream file("data/borrow_records.txt");
   string line;

   while (getline(file, line)) {
      if (line.empty()) continue;

      vector<string> d = split(line);

      borrow_records.push_back({
          stoi(d[0]),   // id
          stoi(d[1]),   // borrower_id
          stoi(d[2]),   // book_id
          stoi(d[3]),   // quantity
          stoi(d[4]),   // status
          d[5],         // borrow_date
          d[6],         // return_date
          d[7],         // return_at
          stoi(d[8]),   // penalty_amt
          d[9],         // created_at
          stoi(d[10]),  // created_by
      });
   }

   return borrow_records;
}

vector<User> loadUsers() {
   vector<User> users;
   ifstream file(USER_FILE);
   if (!file.is_open()) {
      cout << "⚠️ Warning: users.txt not found.\n";
      return users;
   }

   string line;
   while (getline(file, line)) {
      if (line.empty()) continue;

      stringstream ss(line);
      User u;
      string attemptsStr, lockedStr;

      getline(ss, line, '|');
      u.id = stoi(line);
      getline(ss, u.username, '|');
      getline(ss, u.password, '|');
      getline(ss, attemptsStr, '|');
      u.attempts = stoi(attemptsStr);
      getline(ss, lockedStr, '|');
      u.locked = stoi(lockedStr);
      getline(ss, u.type, '|');
      getline(ss, u.created_at, '|');

      users.push_back(u);
   }
   return users;
}

void saveUsers(const vector<User>& users) {
   ofstream file(USER_FILE);
   for (auto& u : users) {
      file << u.id << "|" << u.username << "|" << u.password << "|"
           << u.attempts << "|" << u.locked << "|" << u.type << "|"
           << u.created_at << "\n";
   }
}

bool getUser(const vector<User>& users, const string& username, User& user) {
   for (auto& u : users) {
      if (u.username == username) {
         user = u;
         return true;
      }
   }
   return false;
}

void updateUser(const User& user, vector<User>& users) {
   for (auto& u : users) {
      if (u.id == user.id) {
         u = user;
         saveUsers(users);
         return;
      }
   }
}

void getAdminsListing(const vector<User>& users) {
   cout << "\n===== Admin Listing =====\n\n";
   cout << left << setw(5) << "ID" << setw(15) << "Username" << setw(15)
        << "Role" << setw(10) << "Locked" << setw(10) << "Attempts" << setw(20)
        << "Created At" << endl;

   cout << string(75, '-') << endl;

   for (auto& u : users) {
      if (u.type == "admin" || u.type == "super_admin") {
         cout << left << setw(5) << u.id << setw(15) << u.username << setw(15)
              << u.type << setw(10) << (u.locked ? "YES" : "NO") << setw(10)
              << u.attempts << setw(20) << u.created_at << endl;
      }
   }

   cout << "\nPress Enter to return to menu...";
   cin.ignore();
   cin.get();
}

void addAdmin(vector<User>& users) {
   string username, password, type;
   int roleChoice;

   cout << "\n=== Add New Admin ===\n";
   cout << "Enter new admin username: ";
   cin >> username;

   // check duplicate username
   for (auto& u : users) {
      if (u.username == username) {
         cout << RED << "Error: Username already exists. Try another." << RESET
              << "\n";
         return;
      }
   }

   cout << "Enter password: ";
   cin >> password;

   cout << "\nSelect Role:\n1. super_admin\n2. admin\nChoose: ";
   cin >> roleChoice;

   if (roleChoice != 1 && roleChoice != 2) {
      cout << RED << "Invalid choice." << RESET << "\n";
      return;
   }

   type = (roleChoice == 1 ? "super_admin" : "admin");

   int newId = users.back().id + 1;

   // datetime now
   time_t now = time(0);
   char buffer[20];
   strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
   string createdAt = buffer;

   users.push_back({newId, username, password, 0, 0, type, createdAt});
   saveUsers(users);

   cout << GREEN << "Admin added successfully." << RESET << "\n";

   cout << "\nPress Enter to return to menu...";
   cin.ignore(numeric_limits<streamsize>::max(), '\n');
   cin.get();
}

void resetUserPassword(vector<User>& users) {
   string targetUser;
   cout << "Enter username to reset password: ";
   cin >> targetUser;

   for (auto& u : users) {
      if (u.username == targetUser) {
         cout << "Enter new password: ";
         cin >> u.password;
         u.attempts = 0;
         u.locked = 0;
         saveUsers(users);
         cout << GREEN << "Password reset." << RESET << "\n";
         return;
      }
   }
   cout << RED << "User not found." << RESET << "\n";
}

void changePassword(User& currentUser, vector<User>& users) {
   string oldPass, newPass;
   cout << "\n=== Change Your Password ===\n";
   cout << "Enter current password: ";
   cin >> oldPass;

   if (oldPass != currentUser.password) {
      cout << RED << "Wrong password." << RESET << "\n";
      return;
   }

   cout << "Enter new password: ";
   cin >> newPass;
   currentUser.password = newPass;

   updateUser(currentUser, users);
   cout << GREEN << "Password updated." << RESET << "\n";
}

void adminMenu(User& currentUser, vector<User>& users,
               vector<Borrower>& borrowers, vector<Book>& books,
               vector<BorrowRecord>& borrow_records) {
   // handle admin menu
   while (true) {
      system("clear");

      string title = "ADMIN MENU";
      int leftPad = (WIDTH - title.size()) / 2;
      int rightPad = WIDTH - title.size() - leftPad;

      cout << "+" << string(WIDTH, '-') << "+\n";
      cout << "|" << string(leftPad, ' ') << title << string(rightPad, ' ')
           << "|\n";
      cout << "+" << string(WIDTH, '-') << "+\n";

      vector<pair<int, string>> menu;
      int optionNumber = 1;

      if (currentUser.type == "super_admin") {
         menu.push_back({optionNumber++, "Add Admin"});
         menu.push_back({optionNumber++, "Reset User Password"});
         menu.push_back({optionNumber++, "Admin Listing"});
      }

      menu.push_back({optionNumber++, "Add Book"});
      menu.push_back({optionNumber++, "Add Borrower"});
      menu.push_back({optionNumber++, "Borrow Book"});
      menu.push_back({optionNumber++, "Return Book"});
      menu.push_back({optionNumber++, "Change Password"});
      menu.push_back({optionNumber++, "Quit"});

      for (auto& m : menu) {
         string line = to_string(m.first) + ". " + m.second;
         cout << "| " << left << setw(WIDTH - 2) << line << " |\n";
      }

      cout << "+" << string(WIDTH, '-') << "+\n\n";

      // only handle input checking
      int choice;
      while (true) {
         cout << "Choose: ";
         string input;
         cin >> input;

         bool isNumber = true;
         for (char c : input)
            if (!isdigit(c)) isNumber = false;

         if (!isNumber) {
            cout << RED << "Please enter a number only.\n" << RESET;
            continue; 
         }

         choice = stoi(input);
         if (choice >= 1 && choice <= menu.size()) break;

         cout << RED << "Invalid option. Try again.\n" << RESET;
      }

      string selected = menu[choice - 1].second;

      if (selected == "Add Admin") {
         addAdmin(users);
      } else if (selected == "Reset User Password") {
         resetUserPassword(users);
      } else if (selected == "Admin Listing") {
         getAdminsListing(users);
      } else if (selected == "Add Book") {
         addBook();
      } else if (selected == "Add Borrower") {
         cout << "👤 Add Borrower function here\n";
      } else if (selected == "Borrow Book") {
         cout << "📚 Borrow Book function here\n";
      } else if (selected == "Return Book") {
         returnBook(borrowers, books, borrow_records);
      } else if (selected == "Change Password") {
         changePassword(currentUser, users);
      } else if (selected == "Quit") {
         return;
      }
   }
}

int main() {
   while (true) {
      vector<User> users = loadUsers();
      vector<Borrower> borrowers = loadBorrowers();
      vector<Book> books = loadBooks();
      vector<BorrowRecord> borrow_records = loadBorrowRecords();
      User u;

      string inputUser, inputPass;
      cout << "\nEnter Username: ";
      cin >> inputUser;
      cout << "Enter Password: ";
      cin >> inputPass;

      if (!getUser(users, inputUser, u)) {
         cout << RED << "User not found." << RESET << "\n";

         continue;
      }

      if (u.locked) {
         cout << RED << "Account locked." << RESET << "\n";
         continue;
      }

      if (u.password == inputPass) {
         u.attempts = 0;
         u.locked = 0;
         updateUser(u, users);

         cout << GREEN << "Login Successful, Welcome " << u.username << "!"
              << RESET << "\n";

         if (u.type == "admin" || u.type == "super_admin") {
            adminMenu(u, users, borrowers, books, borrow_records);
         }
      } else {
         u.attempts++;
         if (u.attempts >= 3) u.locked = 1;
         updateUser(u, users);
         cout << RED << "Incorrect password." << RESET << "\n";
      }
   }
}
