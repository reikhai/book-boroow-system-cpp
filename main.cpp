#include <algorithm>   // for transform()
#include <ctime>       // for time functions
#include <filesystem>  // For file existence check
#include <fstream>     // For file operations
#include <iomanip>     // for setw()
#include <iostream>    // For input/output
#include <limits>      // For numeric_limits
#include <sstream>     // For splitting text lines
#include <string>      // For string operations
#include <vector>      // For using vectors
#include <variant>     // For using variant
#include <cctype>      // for isdigit()

using namespace std;

// ESCAPE 字符
#define ESC "\x1B"

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

struct Book {
   int id;
   std::string title, author, isbn;
   int copies;
   std::string created_at;
};

struct BorrowRecord {
   int id, borrower_id, book_id, quantity, status;
   std::string borrow_date, due_date, return_date;
   int penalty_amt;
   std::string created_at;
   int created_by;
};

struct Borrower {
   int id;
   std::string name;
   std::string address;
   std::string contact;
   std::string ic_no;
   std::string created_at;
};

const string USER_FILE = "data/users.txt";

// global data
vector<Borrower> borrowers;
vector<Book> books;
vector<BorrowRecord> borrow_records;

const int WIDTH = 46;

void clearScreen() {
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
   system("cls");  // Windows
#else
   system("clear");  // macOS
#endif
}

// === Rei Khai ===
void updateBooks(vector<Book>& books) {
   ofstream file("data/books.txt");
   for (auto& b : books) {
      file << b.id << "|" << b.title << "|" << b.author << "|" << b.isbn << "|"
           << b.copies << "|" << b.created_at << "\n";
   }
}

void updateBorrowRecords(vector<BorrowRecord>& borrow_records) {
   ofstream file("data/borrow_records.txt");
   for (auto& r : borrow_records) {
      file << r.id << "|" << r.borrower_id << "|" << r.book_id << "|"
           << r.quantity << "|" << r.status << "|" << r.borrow_date << "|"
           << r.due_date << "|" << r.return_date << "|" << r.penalty_amt << "|"
           << r.created_at << "|" << r.created_by << "\n";
   }
}

void returnBook(vector<Borrower>& borrowers, vector<Book>& books,
                vector<BorrowRecord>& borrow_records) {
   cin.ignore(numeric_limits<streamsize>::max(), '\n');

   while (true) {
      string borrowerName, bookTitle;
      cout << YELLOW
           << "\n======== Press ESC then Enter to return ======= " << RESET
           << "\n";

      string title = "Return Book";
      int leftPad = (WIDTH - title.size()) / 2;
      int rightPad = WIDTH - title.size() - leftPad;

      cout << "+" << string(WIDTH, '-') << "+\n";
      cout << "|" << string(leftPad, ' ') << title << string(rightPad, ' ')
           << "|\n";
      cout << "+" << string(WIDTH, '-') << "+\n";

      cout << "Enter borrower name: ";
      getline(cin, borrowerName);

      if (borrowerName == ESC) return;

      cout << "Enter book title (multiple allowed, split by comma): ";
      getline(cin, bookTitle);

      if (bookTitle == ESC) return;

      // ------------------ Split into array ------------------
      string titles[20];  // max 20 books in 1 transaction
      int titleCount = 0;
      string temp = "";

      for (char c : bookTitle) {
         if (c == ',') {
            // trim
            temp.erase(0, temp.find_first_not_of(" \t"));
            temp.erase(temp.find_last_not_of(" \t") + 1);
            if (!temp.empty() && titleCount < 20) titles[titleCount++] = temp;
            temp = "";
         } else {
            temp += c;
         }
      }
      temp.erase(0, temp.find_first_not_of(" \t"));
      temp.erase(temp.find_last_not_of(" \t") + 1);
      if (!temp.empty() && titleCount < 20) titles[titleCount++] = temp;

      if (titleCount == 0) {
         cout << RED << "No valid book title entered." << RESET << "\n";
         continue;  // repeat
      }

      // find borrower
      int borrowerId = -1;
      for (auto& b : borrowers)
         if (b.name == borrowerName) borrowerId = b.id;

      if (borrowerId == -1) {
         cout << RED << "Borrower not found." << RESET << "\n";
         continue;  // repeat
      }

      bool anyReturned = false;

      // ------ Fine summary ------
      int totalFine = 0;
      string lateDetails[20];
      int lateCount = 0;

      // ---------------- Return loop ----------------
      for (int i = 0; i < titleCount; i++) {
         string titleOne = titles[i];

         int bookId = -1;
         for (auto& bk : books)
            if (bk.title == titleOne) bookId = bk.id;

         if (bookId == -1) {
            cout << RED << "Book \"" << titleOne << "\" not found." << RESET
                 << "\n";
            continue;
         }

         BorrowRecord* record = nullptr;
         for (auto& r : borrow_records)
            if (r.borrower_id == borrowerId && r.book_id == bookId &&
                r.status == 0)
               record = &r;

         if (!record) {
            cout << YELLOW << "No active borrow record for \"" << titleOne
                 << "\"." << RESET << "\n";
            continue;
         }

         // Add copies back
         for (auto& bk : books)
            if (bk.id == bookId) bk.copies += record->quantity;

         // Time set
         time_t now = time(0);
         char buf[20];
         strftime(buf, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
         record->return_date = buf;

         // compute late days
         tm due_tm = {};
         stringstream ssDue(record->due_date);
         ssDue >> get_time(&due_tm, "%Y-%m-%d");

         time_t due_time = mktime(&due_tm);
         time_t now_time = time(0);  // current time

         int daysLate = difftime(now_time, due_time) / (60 * 60 * 24);
         int fine = 0;

         if (daysLate > 0) {
            fine = daysLate * 1;
            totalFine += fine;

            if (lateCount < 20)
               lateDetails[lateCount++] =
                   "- " + titleOne + " : " + to_string(daysLate) +
                   " days late (RM " + to_string(fine) + ")";
         }

         record->penalty_amt = fine;
         record->status = 1;
         anyReturned = true;

         cout << GREEN << "Returned: " << titleOne << RESET << "\n";
      }

      // ---------------- Late Summary + Pay ----------------
      if (lateCount > 0) {
         cout << YELLOW << "\nLate Return Summary:\n" << RESET;
         for (int i = 0; i < lateCount; i++) cout << lateDetails[i] << "\n";

         cout << "\nTotal Fine = RM " << totalFine << "\n\n";

         string payChoice;
         cout << "Pay now? (yes/no): ";
         cin >> payChoice;

         if (payChoice != "yes") {
            cout << RED << "Return cancelled. Please settle payment first."
                 << RESET << "\n";
            return;
         }

         cout << GREEN << "Payment received." << RESET << "\n";
      }

      if (!anyReturned) {
         cout << YELLOW << "\nNo books were returned.\n" << RESET;
         continue;  // repeat
      }

      updateBooks(books);
      updateBorrowRecords(borrow_records);
      cout << GREEN
           << "\nAll return processes completed. No overdue records found. \n"
           << RESET;

      cin.ignore();
      cin.get();
      break;
   }
}

// === End ===

// === Adrian ===
// Function to save all books to a file
void saveBooksToFile(const vector<Book>& books) {
   ofstream file("data/books.txt");
   for (const auto& book : books) {
      file << book.id << "|" << book.title << "|" << book.author << "|"
           << book.isbn << "|" << book.copies << "|" << book.created_at << "\n";
   }
   file.close();
}

// Function to get current date as string
string getCurrentDate() {
   time_t now = time(0);
   tm* localTime = localtime(&now);
   char buffer[80];
   strftime(buffer, sizeof(buffer), "%Y-%m-%d", localTime);
   return string(buffer);
}

// Simplified addBook function
void addBook(vector<Book>& books) {
   Book newBook;
   cin.ignore();
   cout << "\n=== Add New Book ===" << endl;
   cout << "Enter book title: ";
   getline(cin, newBook.title);
   cout << "Enter author name: ";
   getline(cin, newBook.author);
   cout << "Enter ISBN: ";
   getline(cin, newBook.isbn);

   // Check for duplicate ISBN (no update logic)
   for (const auto& book : books) {
      if (book.isbn == newBook.isbn) {
         cout << RED << "\n Book with ISBN " << newBook.isbn
              << " already exists in the system.\n"
              << RESET;
         return;
      }
   }

   cout << "Enter number of copies: ";
   while (!(cin >> newBook.copies) || newBook.copies <= 0) {
      cout << RED << "Invalid input. Please enter a positive number: " << RESET;
      cin.clear();
      cin.ignore(10000, '\n');
   }

   newBook.id = books.size() + 1;
   time_t now = time(0);
   tm* ltm = localtime(&now);
   char buffer[20];
   strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
   newBook.created_at = buffer;

   books.push_back(newBook);
   saveBooksToFile(books);

   cout << GREEN << "\n Book added and saved successfully!\n" << RESET;
}
// === End ===

// === Annie ===
// === Save (update) all borrower ===
void updateBorrowers(vector<Borrower>& borrowers) {
   ofstream file("data/borrowers.txt");
   for (auto& b : borrowers) {
      file << b.id << '|' << b.name << '|' << b.address << '|' << b.contact
           << '|' << b.created_at << "\n";
   }
}

// === Add new borrower ===
void addBorrower(vector<Borrower>& borrowers) {
   cin.ignore(numeric_limits<streamsize>::max(), '\n');

   while (true) {
      string title = "Add New Borrower";
      int leftPad = (WIDTH - title.size()) / 2;
      int rightPad = WIDTH - title.size() - leftPad;

      cout << YELLOW << "\n======= Add Borrower =======\n" << RESET;
      cout << "+" << string(WIDTH, '-') << "+\n";
      cout << "|" << string(leftPad, ' ') << title << string(rightPad, ' ')
           << "|\n";
      cout << "+" << string(WIDTH, '-') << "+\n";

      Borrower b;
      b.id = borrowers.empty() ? 1 : borrowers.back().id + 1;

      cout << "Enter Full Name (or type'exit' to return): ";
      getline(cin, b.name);
      if (b.name == "exit" || b.name == "Exit" || b.name == "EXIT") {
         cout << "\nReturning to main menu...\n";
         return;
      }

      cout << "Enter Address (or type'exit' to return): ";
      getline(cin, b.address);
      if (b.address == "exit" || b.address == "Exit" || b.address == "EXIT") {
         cout << "\nReturning to main menu...\n";
         return;
      }

      cout << "Enter Contact Number (or type'exit' to return): ";
      getline(cin, b.contact);
      if (b.contact == "exit" || b.contact == "Exit" || b.contact == "EXIT") {
         cout << "\nReturning to main menu...\n";
         return;
      }

      cout << "Enter ID Number (or type'exit' to return): ";
      getline(cin, b.ic_no);
      if (b.ic_no == "exit" || b.ic_no == "Exit" || b.ic_no == "EXIT") return;
      if (b.ic_no == "exit" || b.ic_no == "Exit" || b.ic_no == "EXIT") {
         cout << "\nReturning to main menu...\n";
         return;
      }

      // time/date
      time_t now = time(0);
      tm* ltm = localtime(&now);
      char buffer[20];
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
      b.created_at = buffer;

      borrowers.push_back(b);
      updateBorrowers(borrowers);

      cout << GREEN << "\nNew borrower added successfully!\n" << RESET;

      string choice;
      cout << "\nAdd another borrower?(Y/N or type'exit' to return): ";
      cin >> choice;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      
      if (choice == "exit" || choice == "Exit" || choice == "EXIT" || 
         choice == "n" || choice == "N") {
         cout << "\nReturning to main menu...\n";
         break;
      }
   }
}

// === Display all borrowers ===
void displayBorrowers(vector<Borrower>& borrowers) {
   cout << YELLOW << "\n======== Borrower List ========\n" << RESET;

   if (borrowers.empty()) {
      cout << RED << "No borrower records found.\n" << RESET;
      return;
   }

   cout << left << setw(20) << "Name" << setw(15) << "Contact" << setw(35)
        << "Address" << "ID\n";
   cout << string(60, '-') << "\n";

   for (auto& b : borrowers) {
      cout << left << setw(20) << b.name << setw(15) << b.contact << setw(35)
           << b.address << b.ic_no << "\n";
   }

   cout << "\nPress Enter to return to menu...";
   cin.ignore();
   cin.get();
}

bool isNumber(const string &s) {
    for (char c : s)
        if (!isdigit(c)) return false;
    return !s.empty();
   }
// === JY ===
void addBorrowRecord(vector<Borrower>& borrowers, vector<Book>& books,
                     vector<BorrowRecord>& borrow_records) {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    vector<pair<int, string>> borrowedBooks;
    string borrowerName;
    int borrowerId = -1;
    cout << YELLOW << "\n======== Borrow Book ========\n" << RESET;

    // Display borrowers
    cout << "\nAvailable Borrowers:\n";
    cout << string(85, '-') << "\n";
    cout << left << setw(5) << "ID" << setw(25) << "Name" << setw(20)
         << "IC Number" << setw(18) << "Contact" << "Address\n";
    cout << string(85, '-') << "\n";

    for (const auto& b : borrowers) {
        cout << left << setw(5) << b.id << setw(25) << b.name << setw(20)
             << b.ic_no << setw(18) << b.contact << b.address << "\n";
    }
    cout << string(85, '-') << "\n";

    // ===== Borrower Input =====
    while (true) {
        string borrowerInput;
        cout << "\nEnter Borrower ID (type 'exit' to return): ";
        cin >> borrowerInput;

        if (borrowerInput == "exit" || borrowerInput == "Exit" || borrowerInput == "EXIT")
            return;

        bool isNumeric = all_of(borrowerInput.begin(), borrowerInput.end(), ::isdigit);
        if (!isNumeric) {
            cout << RED << "Invalid borrower ID!" << RESET << "\n";
            continue;
        }

        borrowerId = stoi(borrowerInput);
        bool found = false;
        for (const auto& b : borrowers) {
            if (b.id == borrowerId) {
                borrowerName = b.name;
                found = true;
                break;
            }
        }

        if (!found) {
            cout << RED << "Invalid borrower ID!" << RESET << "\n";
            continue;
        }

        break; // valid borrower
    }
// === Book Borrowing Section ===
   bool showBookList = true;

   while (true) {
      if (showBookList) {
         cout << "\nAvailable Books:\n";
         cout << string(85, '-') << "\n";
         cout << left << setw(5) << "ID" << setw(35) << "Title" << setw(20)
              << "Author" << "Quantity\n";
         cout << string(85, '-') << "\n";

         for (const auto& b : books) {
            if (b.copies == 0) cout << RED;
            cout << left << setw(5) << b.id << setw(35) << b.title << setw(20)
                 << b.author << b.copies << RESET << "\n";
         }
         cout << string(85, '-') << "\n";
         cout << RED << "Note: Red items are out of stock" << RESET << "\n";
         showBookList = false; // only show once unless borrow again
      }

      string input;
      cout << "\nEnter Book ID (or type 'exit' to finish): ";
      cin >> input;

      if (input == "exit" || input == "Exit" || input == "EXIT") {
         if (!borrowedBooks.empty()){
            break;
         }
         else{
            return;
         }
      }

      bool isNum = all_of(input.begin(), input.end(), ::isdigit);
      if (!isNum) {
         cout << RED << "Book not available!" << RESET << "\n";
         continue;
      }

      int bookId = stoi(input);
      bool found = false, outOfStock = false;
      Book* selectedBook = nullptr;

      for (auto& b : books) {
         if (b.id == bookId) {
            found = true;
            if (b.copies > 0) selectedBook = &b;
            else outOfStock = true;
            break;
         }
      }

      if (!found) {
         cout << RED << "Book not available!" << RESET << "\n";
         continue;
      }
      if (outOfStock) {
         cout << RED << "Book out of stock!" << RESET << "\n";
         continue;
      }

      // === Borrow record ===
      BorrowRecord newRecord;
      newRecord.id = borrow_records.empty() ? 1 : borrow_records.back().id + 1;
      newRecord.borrower_id = borrowerId;
      newRecord.book_id = bookId;
      newRecord.quantity = 1;
      newRecord.status = 0;

      time_t now = time(0);
      tm* ltm = localtime(&now);
      char buffer[20];
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
      newRecord.borrow_date = buffer;

      ltm->tm_mday += 21;
      mktime(ltm);
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
      newRecord.due_date = buffer;

      newRecord.return_date = "NULL";
      newRecord.penalty_amt = 0;
      newRecord.created_at = newRecord.borrow_date;
      newRecord.created_by = 1;

      selectedBook->copies--;
      borrow_records.push_back(newRecord);
      borrowedBooks.push_back({bookId, selectedBook->title});

      char more;
      cout << "\nBorrow another book? (y/n): ";
      cin >> more;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');

      if (tolower(more) == 'y') {
         showBookList = true; // redisplay book list
         continue;
      } else break;
   }

  // === Summary ===
if (!borrowedBooks.empty()) {
    cout << "\n" << string(40, '-') << "\n";
    cout << borrowerName << " borrowed:\n\n";
    for (const auto& book : borrowedBooks)
        cout << book.second << "\n";
    cout << string(40, '-') << "\n";

    updateBorrowRecords(borrow_records);
    updateBooks(books);

    cout << "\nBook issued successfully\nPress Enter to return to menu...";
    
    // clear leftover newline from previous input
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    // wait for user to press Enter only once
    cin.get();
}
}


// === End ===

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
          d[6],         // due_date
          d[7],         // return_date
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
      cout << RED << "Invalid choice.." << RESET << "\n";
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
      clearScreen();

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
      menu.push_back({optionNumber++, "Borrower List"});
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
         addBook(books);
      } else if (selected == "Add Borrower") {
         addBorrower(borrowers);
      } else if (selected == "Borrow Book") {
         addBorrowRecord(borrowers, books, borrow_records);
      } else if (selected == "Return Book") {
         returnBook(borrowers, books, borrow_records);
      } else if (selected == "Change Password") {
         changePassword(currentUser, users);
      } else if (selected == "Quit") {
         return;
      } else if (selected == "Borrower List") {
         displayBorrowers(borrowers);
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

      const string SYSTEM_TITLE = "Book Borrowing System";

      // center title
      int leftPad = (WIDTH - SYSTEM_TITLE.size()) / 2;
      int rightPad = WIDTH - SYSTEM_TITLE.size() - leftPad;

      cout << "+" << string(WIDTH, '-') << "+\n";
      cout << "|" << string(leftPad, ' ') << SYSTEM_TITLE
           << string(rightPad, ' ') << "|\n";
      cout << "+" << string(WIDTH, '-') << "+";

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
