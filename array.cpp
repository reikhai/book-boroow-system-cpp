#include <algorithm>   // for transform()
#include <algorithm>   // for transform
#include <cctype>      // for isdigit()
#include <ctime>       // for time functions
#include <filesystem>  // For file existence check
#include <fstream>     // For file operations
#include <iomanip>     // for setw()
#include <iostream>    // For input/output
#include <limits>      // For numeric_limits
#include <sstream>     // For splitting text lines
#include <string>      // For string operations
#include <variant>     // For using variant
#include <vector>      // For using vectors

using namespace std;

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

// === Data Structures ===
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
   string title, author, isbn;
   int copies;
   string created_at;
};

struct BorrowRecord {
   int id;
   int borrower_id;
   int book_id;
   int quantity;
   int status;
   string borrow_date;
   string due_date;
   string return_date;
   int penalty_amt;
   string created_at;
   int created_by;
};

struct Borrower {
   int id;
   string name;
   string address;
   string contact;
   string ic_no;
   string created_at;
};

// === Global Fixed Arrays ===
Book books[100];
int bookCount = 0;
Borrower borrowers[100];
int borrowerCount = 0;
BorrowRecord borrow_records[200];
int recordCount = 0;
User users[20];
int userCount = 0;

const string USER_FILE = "data/users.txt";

const int WIDTH = 46;

void clearScreen() {
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
   system("cls");  // Windows
#else
   system("clear");  // macOS
#endif
}

bool checkExit(string s) {
   std::transform(s.begin(), s.end(), s.begin(), ::tolower);
   return s == "exit";
}

// === Rei Khai ===
void updateBooks() {
   ofstream file("data/books.txt");
   for (int i = 0; i < bookCount; i++) {
      file << books[i].id << "|" << books[i].title << "|" << books[i].author
           << "|" << books[i].isbn << "|" << books[i].copies << "|"
           << books[i].created_at << "\n";
   }
}

void updateBorrowRecords() {
   ofstream file("data/borrow_records.txt");
   for (int i = 0; i < recordCount; i++) {
      file << borrow_records[i].id << "|" << borrow_records[i].borrower_id
           << "|" << borrow_records[i].book_id << "|"
           << borrow_records[i].quantity << "|" << borrow_records[i].status
           << "|" << borrow_records[i].borrow_date << "|"
           << borrow_records[i].due_date << "|" << borrow_records[i].return_date
           << "|" << borrow_records[i].penalty_amt << "|"
           << borrow_records[i].created_at << "|"
           << borrow_records[i].created_by << "\n";
   }
}

void returnBook() {
   cin.ignore(numeric_limits<streamsize>::max(), '\n');
   clearScreen();

   // ==== BACKUP BEFORE ANY MODIFICATION ====
   Book booksBackup[100];
   BorrowRecord recordsBackup[200];

   for (int i = 0; i < bookCount; i++) booksBackup[i] = books[i];
   for (int i = 0; i < recordCount; i++) recordsBackup[i] = borrow_records[i];

   while (true) {
      string borrowerName, bookTitle;

      string title = "Return Book";
      int leftPad = (WIDTH - title.size()) / 2;
      int rightPad = WIDTH - title.size() - leftPad;

      cout << "+" << string(WIDTH, '-') << "+\n";
      cout << "|" << string(leftPad, ' ') << title << string(rightPad, ' ')
           << "|\n";
      cout << "+" << string(WIDTH, '-') << "+\n";

      cout << "Enter Borrower Name (or type 'exit' to return): ";
      getline(cin, borrowerName);

      if (checkExit(borrowerName)) {
         // === RESTORE BEFORE LEAVING ===
         for (int i = 0; i < bookCount; i++) books[i] = booksBackup[i];
         for (int i = 0; i < recordCount; i++)
            borrow_records[i] = recordsBackup[i];
         cout << "\nReturning to main menu...\n";
         return;
      }

      int borrowerId = -1;
      for (int i = 0; i < borrowerCount; i++)
         if (borrowers[i].name == borrowerName) borrowerId = borrowers[i].id;

      if (borrowerId == -1) {
         cout << RED << "Borrower not found.\n" << RESET;
         continue;
      }

      cout << "Enter book title (multiple allowed, split by comma) (or "
              "type'exit' to return): ";
      getline(cin, bookTitle);

      if (checkExit(bookTitle)) {
         // === RESTORE BEFORE LEAVING ===
         for (int i = 0; i < bookCount; i++) books[i] = booksBackup[i];
         for (int i = 0; i < recordCount; i++)
            borrow_records[i] = recordsBackup[i];
         cout << "\nReturning to main menu...\n";
         return;
      }

      string titles[20];
      int titleCount = 0, lateCount = 0;
      string temp = "";

      for (char c : bookTitle) {
         if (c == ',') {
            temp.erase(0, temp.find_first_not_of(" \t"));
            temp.erase(temp.find_last_not_of(" \t") + 1);
            if (!temp.empty()) titles[titleCount++] = temp;
            temp.clear();
         } else
            temp += c;
      }
      if (!temp.empty()) {
         temp.erase(0, temp.find_first_not_of(" \t"));
         temp.erase(temp.find_last_not_of(" \t") + 1);
         titles[titleCount++] = temp;
      }

      if (titleCount == 0) {
         cout << RED << "No valid book title entered." << RESET << "\n";
         continue;
      }

      // ===== CHECK IF ALL TITLES EXIST =====
      bool allValid = true;
      for (int i = 0; i < titleCount; i++) {
         string titleOne = titles[i];
         bool exists = false;

         for (int j = 0; j < bookCount; j++) {
            if (books[j].title == titleOne) {
               exists = true;
               break;
            }
         }

         if (!exists) {
            cout << RED << "Book \"" << titleOne << "\" not found. Try again.\n"
                 << RESET;
            allValid = false;
            break;
         }
      }

      if (!allValid) {
         cout << RED << "Please re-enter book titles.\n" << RESET;
         continue;  // go back to ask title input again
      }

      bool anyReturned = false;
      int totalFine = 0;
      string lateDetails[20];

      for (int i = 0; i < titleCount; i++) {
         string titleOne = titles[i];

         int bookIdx = -1;
         for (int j = 0; j < bookCount; j++)
            if (books[j].title == titleOne) bookIdx = j;

         if (bookIdx == -1) {
            cout << RED << "Book \"" << titleOne << "\" not found.\n" << RESET;
            continue;
         }

         int recIdx = -1;
         for (int r = 0; r < recordCount; r++)
            if (borrow_records[r].borrower_id == borrowerId &&
                borrow_records[r].book_id == books[bookIdx].id &&
                borrow_records[r].status == 0)
               recIdx = r;

         if (recIdx == -1) {
            cout << YELLOW << "No active borrow record for \"" << titleOne
                 << "\".\n"
                 << RESET;
            continue;
         }

         books[bookIdx].copies += borrow_records[recIdx].quantity;

         time_t now = time(0);
         char buf[20];
         strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
         borrow_records[recIdx].return_date = buf;

         tm due_tm = {};
         stringstream ss(borrow_records[recIdx].due_date);
         ss >> get_time(&due_tm, "%Y-%m-%d %H:%M:%S");
         time_t due_time = mktime(&due_tm);

         int daysLate = difftime(now, due_time) / (60 * 60 * 24);
         int fine = daysLate > 0 ? daysLate : 0;

         borrow_records[recIdx].penalty_amt = fine;
         borrow_records[recIdx].status = 1;
         anyReturned = true;

         if (fine > 0 && lateCount < 20) {
            totalFine += fine;
            lateDetails[lateCount++] = "- " + titleOne + ": " +
                                       to_string(daysLate) + " days late (RM " +
                                       to_string(fine) + ")";
         }

         cout << GREEN << "Returned: " << titleOne << RESET << "\n";
      }

      if (lateCount > 0) {
         cout << YELLOW << "\nLate Return Summary:\n" << RESET;
         for (int i = 0; i < lateCount; i++) cout << lateDetails[i] << "\n";
         cout << "\nTotal Fine: RM " << totalFine << "\n";

         cout << "Pay now? (y/n): ";
         string pay;
         cin >> pay;
         cin.ignore(numeric_limits<streamsize>::max(), '\n');

         if (pay != "y" && pay != "Y") {
            // === RESTORE IF USER REFUSES TO PAY ===
            for (int i = 0; i < bookCount; i++) books[i] = booksBackup[i];
            for (int i = 0; i < recordCount; i++)
               borrow_records[i] = recordsBackup[i];
            cout << RED << "Return cancelled. Changes reverted.\n" << RESET;
            return;
         }

         cout << GREEN << "Payment received.\n" << RESET;
      }

      updateBooks();
      updateBorrowRecords();
      cout << GREEN << "\nReturn process completed.\n" << RESET;

      cout << "\nReturn another? (y/n): ";
      string again;
      cin >> again;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      if (again != "y" && again != "Y") return;
      clearScreen();
   }
}

void bookInventory() {
   clearScreen();

   cout << YELLOW << "\n============ List of Books Inventory ============\n"
        << RESET;

   if (bookCount == 0) {
      cout << RED << "\nNo books in inventory.\n" << RESET;
      cin.get();
      return;
   }

   cout << left << setw(6) << "ID" << setw(32) << "Title" << setw(25)
        << "Author" << setw(18) << "ISBN" << setw(15) << "Total" << setw(15)
        << "Borrowed" << setw(15) << "Available" << "\n";

   cout << string(120, '-') << "\n";

   for (int i = 0; i < bookCount; i++) {
      int borrowedCount = 0;

      for (int j = 0; j < recordCount; j++)
         if (borrow_records[j].book_id == books[i].id &&
             borrow_records[j].status == 0)
            borrowedCount += borrow_records[j].quantity;

      int available = books[i].copies - borrowedCount;
      if (available < 0) available = 0;

      cout << left << setw(6) << books[i].id << setw(30) << books[i].title
           << setw(22) << books[i].author << setw(25) << books[i].isbn
           << setw(15) << books[i].copies << setw(15) << borrowedCount;

      if (available == 0)
         cout << RED << setw(15) << available << RESET << "\n";
      else
         cout << GREEN << setw(15) << available << RESET << "\n";
   }

   cout << "\n" << string(120, '-') << "\n";
   cout << "Press Enter to return...";

   cin.ignore(numeric_limits<streamsize>::max(), '\n');
   cin.get();
}
// === End ===

// === Adrian ===
void addBook() {
   cin.ignore(numeric_limits<streamsize>::max(), '\n');

   // ==== Backup current state in case user exits ====
   Book booksBackup[100];
   int bookCountBackup = bookCount;
   for (int i = 0; i < bookCount; i++) booksBackup[i] = books[i];

   Book newBook;

   cout << "\n=== Add New Book ===\n";

   cout << "Enter Book Title (or type 'exit' to return): ";
   getline(cin, newBook.title);
   if (checkExit(newBook.title)) {
      // Restore original data
      for (int i = 0; i < bookCountBackup; i++) books[i] = booksBackup[i];
      bookCount = bookCountBackup;
      cout << "\nReturning to main menu...\n";
      return;
   }

   cout << "Enter Author Name (or type 'exit' to return): ";
   getline(cin, newBook.author);
   if (checkExit(newBook.author)) {
      for (int i = 0; i < bookCountBackup; i++) books[i] = booksBackup[i];
      bookCount = bookCountBackup;
      cout << "\nReturning to main menu...\n";
      return;
   }

   while (true) {
      cout << "Enter ISBN (or type 'exit' to return): ";
      getline(cin, newBook.isbn);

      if (checkExit(newBook.isbn)) {
         for (int i = 0; i < bookCountBackup; i++) books[i] = booksBackup[i];
         bookCount = bookCountBackup;
         cout << "\nReturning to main menu...\n";

         return;
      }

      bool duplicate = false;
      for (int i = 0; i < bookCount; i++) {
         if (books[i].isbn == newBook.isbn) {
            duplicate = true;
            break;
         }
      }

      if (duplicate) {
         cout << RED << "\nBook with ISBN \"" << newBook.isbn
              << "\" already exists. Please enter a different ISBN.\n\n"
              << RESET;
         continue;
      }

      break;
   }

   cout << "Enter number of copies (or type 'exit' to return): ";

   while (true) {
      string copies_input;
      cin >> copies_input;

      if (checkExit(copies_input)) {
         // restore data
         for (int i = 0; i < bookCountBackup; i++) books[i] = booksBackup[i];
         bookCount = bookCountBackup;

         cout << "\nReturning to main menu...\n";
         cin.ignore(numeric_limits<streamsize>::max(), '\n');
         return;
      }

      // check numeric
      bool isNum = all_of(copies_input.begin(), copies_input.end(), ::isdigit);
      if (!isNum) {
         cout << RED << "Invalid input. Enter a positive number: " << RESET;
         continue;
      }

      newBook.copies = stoi(copies_input);
      if (newBook.copies <= 0) {
         cout << RED << "Copies must be greater than 0: " << RESET;
         continue;
      }

      break;  // valid number, exit loop
   }

   newBook.id = (bookCount == 0 ? 1 : books[bookCount - 1].id + 1);

   time_t now = time(0);
   char buffer[20];
   strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
   newBook.created_at = buffer;

   books[bookCount++] = newBook;
   updateBooks();

   cout << GREEN << "\nBook added and saved successfully!\n" << RESET;
}
// === End ===

// === Annie ===
void updateBorrowers() {
   ofstream file("data/borrowers.txt");
   for (int i = 0; i < borrowerCount; i++) {
      file << borrowers[i].id << "|" << borrowers[i].name << "|"
           << borrowers[i].address << "|" << borrowers[i].contact << "|"
           << borrowers[i].ic_no << "|" << borrowers[i].created_at << "\n";
   }
}

void addBorrower() {
   cin.ignore(numeric_limits<streamsize>::max(), '\n');

   // --- Backup original borrowers state (for exit rollback) ---
   Borrower borrowersBackup[100];
   int borrowerCountBackup = borrowerCount;
   for (int i = 0; i < borrowerCount; i++) borrowersBackup[i] = borrowers[i];

   while (true) {
      string title = "Add New Borrower";
      int leftPad = (WIDTH - title.size()) / 2;
      int rightPad = WIDTH - title.size() - leftPad;

      cout << "+" << string(WIDTH, '-') << "+\n";
      cout << "|" << string(leftPad, ' ') << title << string(rightPad, ' ')
           << "|\n";
      cout << "+" << string(WIDTH, '-') << "+\n";

      Borrower b;
      b.id = (borrowerCount == 0 ? 1 : borrowers[borrowerCount - 1].id + 1);

      while(true) {
         cout << "Enter Full Name (or 'exit' to return): ";
         getline(cin, b.name);
         
         if (checkExit(b.name)) {
            // Restore original data
            borrowerCount = borrowerCountBackup;
            for (int i = 0; i < borrowerCount; i++)
               borrowers[i] = borrowersBackup[i];
            cout << "\nReturning to main menu...\n";
            return;
         }

         if (b.name.empty()){
            cout << RED << "Error: Name cannot be empty!\n" << RESET;
            continue;
         }
         break;
      }

      while (true) {
         cout << "Enter Address (or 'exit' to return): ";
         getline(cin, b.address);
         
         if (checkExit(b.address)) {
            borrowerCount = borrowerCountBackup;
            for (int i = 0; i < borrowerCount; i++)
               borrowers[i] = borrowersBackup[i];
            cout << "\nReturning to main menu...\n";
            return;
         }
         if (b.address.empty()){
            cout << RED << "Error: Address cannot be empty!\n" << RESET;
            continue;
         }
         break;
      }

      while (true) {
         cout << "Enter Contact Number (or 'exit' to return): ";
         getline(cin, b.contact);
         
         if (checkExit(b.contact)) {
            borrowerCount = borrowerCountBackup;
            for (int i = 0; i < borrowerCount; i++)
               borrowers[i] = borrowersBackup[i];
            cout << "\nReturning to main menu...\n";
            return;
         }
         if (b.contact.empty()){
            cout << RED << "Error: Contact Number cannot be empty!\n" << RESET;
            continue;
         }
         break;
      }

      while (true){
         cout << "Enter IC Number (or 'exit' to return): ";
         getline(cin, b.ic_no);
         
         if (checkExit(b.ic_no)) {
            borrowerCount = borrowerCountBackup;
            for (int i = 0; i < borrowerCount; i++)
               borrowers[i] = borrowersBackup[i];
            cout << "\nReturning to main menu...\n";
            return;
         }
       if (b.ic_no.empty()){
            cout << RED << "Error: IC Number cannot be empty!\n" << RESET;
            continue;
         }
         break;
      }

      // Timestamp
      time_t now = time(0);
      char buffer[20];
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
      b.created_at = buffer;

      // Save and write to file (only when adding is confirmed)
      borrowers[borrowerCount++] = b;
      updateBorrowers();

      cout << GREEN << "\nNew borrower added successfully!\n" << RESET;

      string choice;
      cout << "\nAdd another borrower?(Y/N or type'exit' to return): ";
      cin >> choice;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');

      if (checkExit(choice) || choice == "n" || choice == "N") {
         cout << "\nReturning to main menu...\n";
         break;
      }
   }
}

void displayBorrowersWithBooks() {
   cout << YELLOW << "\n======== List of Borrowed Books ========\n" << RESET;

   if (borrowerCount == 0) {
      cout << RED << "No borrower records found!" << RESET << endl;
      cin.get();
      return;
   }

   for (int i = 0; i < borrowerCount; i++) {
      cout << "\n" << string(90, '-') << "\n";
      cout << GREEN << "Borrower ID: " << borrowers[i].id << RESET << endl;
      cout << "Name    : " << borrowers[i].name << endl;
      cout << "Address : " << borrowers[i].address << endl;
      cout << "Contact : " << borrowers[i].contact << endl;
      cout << "IC Number   : " << borrowers[i].ic_no << endl;

      bool hasBorrowed = false;

      for (int j = 0; j < recordCount; j++) {
         if (borrow_records[j].borrower_id == borrowers[i].id &&
             borrow_records[j].status == 0) {
            hasBorrowed = true;
            string bookTitle = "Unknown";

            for (int k = 0; k < bookCount; k++)
               if (books[k].id == borrow_records[j].book_id)
                  bookTitle = books[k].title;

            cout << "   - " << bookTitle
                 << " (Borrowed: " << borrow_records[j].borrow_date
                 << ", Due: " << borrow_records[j].due_date << ")\n";
         }
      }

      if (!hasBorrowed) {
         cout << RED << "No books currently borrowed." << RESET << endl;
      }
   }

   cout << "\n" << string(90, '-') << "\n";
   cout << "Press Enter to return to the main menu...";
   cin.ignore(numeric_limits<streamsize>::max(), '\n');
   cin.get();
}
// === End ===

// === JY ===
void addBorrowRecord() {
   cin.ignore(numeric_limits<streamsize>::max(), '\n');

   // ===== BACKUP before any changes =====
   Book booksBackup[100];
   BorrowRecord recordsBackup[200];
   int recordCountBackup = recordCount;
   for (int i = 0; i < bookCount; i++) booksBackup[i] = books[i];
   for (int i = 0; i < recordCount; i++) recordsBackup[i] = borrow_records[i];

   vector<pair<int, string>> borrowedBooks;
   int borrowerId = -1;
   string borrowerName;

   cout << YELLOW << "\n======== Borrow Book ========\n" << RESET;

   // Display borrowers
   cout << "\nAvailable Borrowers:\n";
   cout << string(85, '-') << "\n";
   cout << left << setw(5) << "ID" << setw(25) << "Name" << setw(20)
        << "IC Number" << setw(18) << "Contact" << "Address\n";
   cout << string(85, '-') << "\n";

   for (int i = 0; i < borrowerCount; i++) {
      cout << left << setw(5) << borrowers[i].id << setw(25)
           << borrowers[i].name << setw(20) << borrowers[i].ic_no << setw(18)
           << borrowers[i].contact << borrowers[i].address << "\n";
   }
   cout << string(85, '-') << "\n";

   // Borrower Input
   while (true) {
      string borrowerInput;
      cout << "\nEnter Borrower ID (type 'exit' to return): ";
      cin >> borrowerInput;

      if (checkExit(borrowerInput)) {
         // ===== RESTORE on exit =====
         recordCount = recordCountBackup;
         for (int i = 0; i < bookCount; i++) books[i] = booksBackup[i];
         for (int i = 0; i < recordCount; i++)
            borrow_records[i] = recordsBackup[i];
         cout << "\nReturning to main menu...\n";
         return;
      }

      bool isNumeric =
          all_of(borrowerInput.begin(), borrowerInput.end(), ::isdigit);
      if (!isNumeric) {
         cout << RED << "Invalid borrower ID!" << RESET << "\n";
         continue;
      }

      borrowerId = stoi(borrowerInput);
      bool found = false;
      for (int i = 0; i < borrowerCount; i++) {
         if (borrowers[i].id == borrowerId) {
            borrowerName = borrowers[i].name;
            found = true;
            break;
         }
      }

      if (!found) {
         cout << RED << "Invalid borrower ID!" << RESET << "\n";
         continue;
      }
      break;  // valid borrower
   }

   bool showBookList = true;

   while (true) {
      if (showBookList) {
         cout << "\nAvailable Books:\n";
         cout << string(85, '-') << "\n";
         cout << left << setw(5) << "ID" << setw(35) << "Title" << setw(20)
              << "Author" << "Quantity\n";
         cout << string(85, '-') << "\n";

         for (int i = 0; i < bookCount; i++) {
            if (books[i].copies == 0) cout << RED;
            cout << left << setw(5) << books[i].id << setw(35) << books[i].title
                 << setw(20) << books[i].author << books[i].copies << RESET
                 << "\n";
         }
         cout << string(85, '-') << "\n";
         cout << RED << "Note: Red items mean out of stock." << RESET << "\n";
         showBookList = false;  // only show once unless borrow again
      }

      string input;
      cout << "\nEnter Book ID (or type 'exit' to finish): ";
      cin >> input;

      if (checkExit(input)) {
         if (!borrowedBooks.empty()) {
            break;  // go summary (save)
         } else {
            // ===== RESTORE on exit WITHOUT borrowing =====
            recordCount = recordCountBackup;
            for (int i = 0; i < bookCount; i++) books[i] = booksBackup[i];
            for (int i = 0; i < recordCount; i++)
               borrow_records[i] = recordsBackup[i];
            return;
         }
      }

      bool isNum = all_of(input.begin(), input.end(), ::isdigit);
      if (!isNum) {
         cout << RED << "Book not available!" << RESET << "\n";
         continue;
      }

      int bookId = stoi(input);
      int idx = -1;

      for (int i = 0; i < bookCount; i++)
         if (books[i].id == bookId) idx = i;

      if (idx == -1) {
         cout << RED << "Book not available!" << RESET << "\n";
         continue;
      }

      if (books[idx].copies == 0) {
         cout << RED << "Book out of stock!" << RESET << "\n";
         continue;
      }

      // === Borrow record ===
      BorrowRecord newRecord;
      newRecord.id =
          (recordCount == 0 ? 1 : borrow_records[recordCount - 1].id + 1);
      newRecord.borrower_id = borrowerId;
      newRecord.book_id = bookId;
      newRecord.quantity = 1;
      newRecord.status = 0;

      time_t now = time(0);
      char buffer[20];
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
      newRecord.borrow_date = buffer;

      tm* ltm = localtime(&now);
      ltm->tm_mday += 21;
      mktime(ltm);
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
      newRecord.due_date = buffer;

      newRecord.return_date = "NULL";
      newRecord.penalty_amt = 0;
      newRecord.created_at = newRecord.borrow_date;
      newRecord.created_by = 1;

      books[idx].copies--;
      borrow_records[recordCount++] = newRecord;

      borrowedBooks.push_back({bookId, books[idx].title});

      cout << GREEN << "\nBook added to borrow list.\n" << RESET;

      char more;
      cout << "\nBorrow another book? (y/n): ";
      cin >> more;

      if (tolower(more) == 'y') {
         showBookList = true;  // redisplay book list
         continue;
      }
      break;
   }

   // === Summary ===
   cout << "\n" << string(40, '-') << "\n";
   cout << borrowerName << " borrowed:\n\n";
   for (auto& b : borrowedBooks) cout << " - " << b.second << "\n";
   cout << string(40, '-') << "\n";

   // ===== FINAL SAVE (only here) =====
   updateBooks();
   updateBorrowRecords();

   cout << GREEN << "\nBook issued successfully!\n" << RESET;
   cout << "\nPress Enter to return to menu...";
   cin.ignore(numeric_limits<streamsize>::max(), '\n');
   cin.get();
}
// === End ===

vector<string> split(const string& line, char delimiter = '|') {
   vector<string> parts;
   stringstream ss(line);
   string part;
   while (getline(ss, part, delimiter)) parts.push_back(part);
   return parts;
}

// Load Borrowers
void loadBorrowers() {
   borrowerCount = 0;
   ifstream file("data/borrowers.txt");
   string line;

   while (getline(file, line)) {
      vector<string> d = split(line);
      borrowers[borrowerCount].id = stoi(d[0]);
      borrowers[borrowerCount].name = d[1];
      borrowers[borrowerCount].address = d[2];
      borrowers[borrowerCount].contact = d[3];
      borrowers[borrowerCount].ic_no = d[4];
      borrowers[borrowerCount].created_at = d[5];
      borrowerCount++;
   }
}

// Load Books
void loadBooks() {
   bookCount = 0;
   ifstream file("data/books.txt");
   string line;

   while (getline(file, line)) {
      vector<string> d = split(line);
      books[bookCount].id = stoi(d[0]);
      books[bookCount].title = d[1];
      books[bookCount].author = d[2];
      books[bookCount].isbn = d[3];
      books[bookCount].copies = stoi(d[4]);
      books[bookCount].created_at = d[5];
      bookCount++;
   }
}

// Load Borrow Records
void loadBorrowRecords() {
   recordCount = 0;
   ifstream file("data/borrow_records.txt");
   string line;

   while (getline(file, line)) {
      if (line.empty()) continue;
      vector<string> d = split(line);

      borrow_records[recordCount].id = stoi(d[0]);
      borrow_records[recordCount].borrower_id = stoi(d[1]);
      borrow_records[recordCount].book_id = stoi(d[2]);
      borrow_records[recordCount].quantity = stoi(d[3]);
      borrow_records[recordCount].status = stoi(d[4]);
      borrow_records[recordCount].borrow_date = d[5];
      borrow_records[recordCount].due_date = d[6];
      borrow_records[recordCount].return_date = d[7];
      borrow_records[recordCount].penalty_amt = stoi(d[8]);
      borrow_records[recordCount].created_at = d[9];
      borrow_records[recordCount].created_by = stoi(d[10]);

      recordCount++;
   }
}

// Load Users
void loadUsers() {
   userCount = 0;
   ifstream file(USER_FILE);
   string line;

   while (getline(file, line)) {
      if (line.empty()) continue;
      vector<string> d = split(line);
      users[userCount].id = stoi(d[0]);
      users[userCount].username = d[1];
      users[userCount].password = d[2];
      users[userCount].attempts = stoi(d[3]);
      users[userCount].locked = stoi(d[4]);
      users[userCount].type = d[5];
      users[userCount].created_at = d[6];
      userCount++;
   }
}

void saveUsers() {
   ofstream file(USER_FILE);
   for (int i = 0; i < userCount; i++) {
      file << users[i].id << "|" << users[i].username << "|"
           << users[i].password << "|" << users[i].attempts << "|"
           << users[i].locked << "|" << users[i].type << "|"
           << users[i].created_at << "\n";
   }
}

bool getUser(const string& username, User& userOut) {
   for (int i = 0; i < userCount; i++) {
      if (users[i].username == username) {
         userOut = users[i];
         return true;
      }
   }
   return false;
}

void updateUser(const User& user) {
   for (int i = 0; i < userCount; i++) {
      if (users[i].id == user.id) {
         users[i] = user;
         saveUsers();
         return;
      }
   }
}

void getAdminsListing() {
   cout << "\n===== Admin Listing =====\n\n";

   cout << left << setw(5) << "ID" << setw(15) << "Username" << setw(15)
        << "Role" << setw(10) << "Locked" << setw(10) << "Attempts" << setw(20)
        << "Created At" << endl;

   cout << string(75, '-') << endl;

   for (int i = 0; i < userCount; i++) {
      if (users[i].type == "admin" || users[i].type == "super_admin") {
         cout << left << setw(5) << users[i].id << setw(15) << users[i].username
              << setw(15) << users[i].type << setw(10)
              << (users[i].locked ? "YES" : "NO") << setw(10)
              << users[i].attempts << setw(20) << users[i].created_at << endl;
      }
   }

   cout << "\nPress Enter to return...";
   cin.ignore();
   cin.get();
}

void addAdmin() {
   string username, password;
   int roleChoice;

   while (true) {
      cout << "\n=== Add New Admin ===\n";
      cout << "Enter new admin username (or type'exit' to return): ";
      cin >> username;

      if (checkExit(username)) {
         cout << "\nReturning to main menu...\n";
         return;
      }

      bool exists = false;
      for (int i = 0; i < userCount; i++) {
         if (users[i].username == username) {
            exists = true;
            break;
         }
      }

      if (exists) {
         cout << RED
              << "Username already exists. Please enter a different username.\n"
              << RESET;
         continue;
      }

      cout << "Enter password (or type'exit' to return): ";
      cin >> password;

      if (checkExit(password)) {
         cout << "\nReturning to main menu...\n";
         return;
      }

      string type;
      while (true) {
         cout << "\nSelect Role:\n1. super_admin\n2. admin\n3. exit\nChoose: ";

         if (!(cin >> roleChoice)) {  // is not a number
            cin.clear();              // clear error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << RED << "Invalid input. Please enter a number (1 or 2).\n"
                 << RESET;
            continue;
         }

         if (roleChoice == 1) {
            type = "super_admin";
            break;
         } else if (roleChoice == 2) {
            type = "admin";
            break;
         } else if (roleChoice == 3) {
            cout << "\nReturning to main menu...\n";
            return;
         }

         cout << RED << "Invalid choice. Please enter 1 , 2 or 3.\n" << RESET;
      }

      cin.ignore(numeric_limits<streamsize>::max(), '\n');

      User u;
      u.id = (userCount == 0 ? 1 : users[userCount - 1].id + 1);
      u.username = username;
      u.password = password;
      u.attempts = 0;
      u.locked = 0;
      u.type = type;

      time_t now = time(0);
      char buffer[20];
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
      u.created_at = buffer;

      users[userCount++] = u;
      saveUsers();

      cout << GREEN << "Admin added successfully.\n" << RESET;

      string choice;
      cout << "\nAdd another admin?(Y/N or type'exit' to return): ";
      cin >> choice;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');

      if (checkExit(choice) || choice == "n" || choice == "N") {
         cout << "\nReturning to main menu...\n";
         break;
      }
   }
}

void resetUserPassword() {
   // --- Backup for rollback on exit ---
   User usersBackup[20];
   for (int i = 0; i < userCount; i++) usersBackup[i] = users[i];

   while (true) {
      cout << "\n=== Reset User Password ===\n";
      cout << "Enter username to reset password (or type 'exit' to return): ";

      string targetUser;
      cin >> targetUser;

      if (checkExit(targetUser)) {
         // restore backup
         for (int i = 0; i < userCount; i++) users[i] = usersBackup[i];
         cout << "\nReturning to main menu...\n";
         return;
      }

      // find user
      int idx = -1;
      for (int i = 0; i < userCount; i++) {
         if (users[i].username == targetUser) {
            idx = i;
            break;
         }
      }

      if (idx == -1) {
         cout << RED << "User not found. Please try again.\n" << RESET;
         continue;
      }

      // ---- Found User ----
      cout << "Enter new password (or type 'exit' to return): ";
      string newPass;
      cin >> newPass;

      if (checkExit(newPass)) {
         // restore backup
         for (int i = 0; i < userCount; i++) users[i] = usersBackup[i];
         cout << "\nReturning to main menu...\n";
         return;
      }

      users[idx].password = newPass;
      users[idx].attempts = 0;
      users[idx].locked = 0;

      saveUsers();
      cout << GREEN << "Password reset successfully.\n" << RESET;

      cout << "\nPress ENTER to return to main menu...\n";
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      cin.get();
      return;
   }
}

void changePassword(User& currentUser) {
   // --- Backup for rollback on exit ---
   User usersBackup[20];

   for (int i = 0; i < userCount; i++)
      users[i] = users[i];
   for (int i = 0; i < userCount; i++) usersBackup[i] = users[i];

   User currentUserBackup = currentUser;

   cout << "\n=== Change Your Password ===\n";

   while (true) {
      string oldPass;
      cout << "Enter current password (or type 'exit' to return): ";
      cin >> oldPass;

      if (checkExit(oldPass)) {
         // restore backup
         currentUser = currentUserBackup;
         for (int i = 0; i < userCount; i++) users[i] = usersBackup[i];
         cout << "\nReturning to main menu...\n";
         return;
      }

      if (oldPass != currentUser.password) {
         cout << RED << "Wrong password. Please try again.\n" << RESET;
         continue;
      }

      break;
   }

   while (true) {
      string newPass;
      cout << "Enter new password (or type 'exit' to return): ";
      cin >> newPass;

      if (checkExit(newPass)) {
         // restore backup
         currentUser = currentUserBackup;
         for (int i = 0; i < userCount; i++) users[i] = usersBackup[i];
         cout << "\nReturning to main menu...\n";
         return;
      }

      currentUser.password = newPass;
      for (int i = 0; i < userCount; i++) {
         if (users[i].id == currentUser.id) {
            users[i].password = newPass;
            break;
         }
      }

      saveUsers();
      cout << GREEN << "Password updated successfully.\n" << RESET;
      cout << "\nPress Enter to return to main menu...";
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      cin.get();
      return;
   }
}

void adminMenu(User& currentUser) {
   // handle admin menu
   while (true) {
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
      menu.push_back({optionNumber++, "List of Borrowed Books"});
      menu.push_back({optionNumber++, "List of Book Inventory"});
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

      if (selected == "Add Admin")
         addAdmin();
      else if (selected == "Reset User Password")
         resetUserPassword();
      else if (selected == "Admin Listing")
         getAdminsListing();
      else if (selected == "Add Book")
         addBook();
      else if (selected == "Add Borrower")
         addBorrower();
      else if (selected == "Borrow Book")
         addBorrowRecord();
      else if (selected == "Return Book")
         returnBook();
      else if (selected == "List of Borrowed Books")
         displayBorrowersWithBooks();
      else if (selected == "List of Book Inventory")
         bookInventory();
      else if (selected == "Change Password")
         changePassword(currentUser);
      else if (selected == "Quit")
         return;
   }
}

int main() {
   loadUsers();
   loadBorrowers();
   loadBooks();
   loadBorrowRecords();

   while (true) {
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

      if (!getUser(inputUser, u)) {
         cout << RED << "User not found.\n" << RESET;
         continue;
      }

      if (u.locked) {
         cout << RED << "Account is locked.\n" << RESET;
         continue;
      }

      if (u.password == inputPass) {
         u.attempts = 0;
         u.locked = false;
         updateUser(u);

         cout << GREEN << "Login Successful. Welcome, " << u.username << "!\n"
              << RESET;
         if (u.type == "super_admin" || u.type == "admin") adminMenu(u);
      } else {
         u.attempts++;
         if (u.attempts >= 3) u.locked = true;
         updateUser(u);
         cout << RED << "Incorrect password.\n" << RESET;
      }
   }
}
