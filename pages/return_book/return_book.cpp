#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../../struct/book.h"
#include "../../struct/borrow_record.h"
#include "../../struct/borrower.h"
using namespace std;

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

const int WIDTH = 46;

// ESCAPE 字符
#define ESC "\x1B"

void saveBooks(vector<Book>& books) {
   ofstream file("data/books.txt");
   for (auto& b : books) {
      file << b.id << "|" << b.title << "|" << b.author << "|" << b.isbn << "|"
           << b.copies << "|" << b.created_at << "\n";
   }
}

void saveBorrowRecords(vector<BorrowRecord>& borrow_records) {
   ofstream file("data/borrow_records.txt");
   for (auto& r : borrow_records) {
      file << r.id << "|" << r.borrower_id << "|" << r.book_id << "|"
           << r.quantity << "|" << r.status << "|" << r.borrow_date << "|"
           << r.return_date << "|" << r.return_at << "|" << r.penalty_amt << "|"
           << r.created_at << "|" << r.created_by << "\n";
   }
}

void returnBook(vector<Borrower>& borrowers, vector<Book>& books,
                vector<BorrowRecord>& borrow_records) {
   cin.ignore(numeric_limits<streamsize>::max(), '\n');

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

   // ESC back to menu
   if (borrowerName == ESC) {
      return;
   }

   cout << "Enter book title: ";

   // ESC back to menu
   if (bookTitle == ESC) {
      return;
   }
   getline(cin, bookTitle);

   // find borrower
   int borrowerId = -1;
   for (auto& b : borrowers)
      if (b.name == borrowerName) borrowerId = b.id;

   if (borrowerId == -1) {
      cout << RED << "Borrower not found." << RESET << "\n";
      return;
   }

   // find book
   int bookId = -1;
   for (auto& bk : books)
      if (bk.title == bookTitle) bookId = bk.id;

   if (bookId == -1) {
      cout << RED << "Book not found." << RESET << "\n";
      return;
   }

   // 找未归还记录 (status == 0)
   BorrowRecord* record = nullptr;
   for (auto& r : borrow_records)
      if (r.borrower_id == borrowerId && r.book_id == bookId && r.status == 0)
         record = &r;

   if (!record) {
      cout << YELLOW << "No active borrow record." << RESET << "\n";
      return;
   }

   for (auto& bk : books)
      if (bk.id == bookId) bk.copies += record->quantity;

   time_t now = time(0);
   char buf[20];
   strftime(buf, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
   record->return_at = buf;

   // 转换字符串日期为 tm 结构
   tm borrow_tm = {}, return_tm = {};
   stringstream ss1(record->borrow_date);
   stringstream ss2(record->return_date);

   ss1 >> get_time(&borrow_tm, "%Y-%m-%d");
   ss2 >> get_time(&return_tm, "%Y-%m-%d");

   // convert to time_t
   time_t borrow_time = mktime(&borrow_tm);
   time_t return_time = mktime(&return_tm);

   // count days
   int daysLate = difftime(return_time, borrow_time) / (60 * 60 * 24);
   int fine = 0;
   if (daysLate > 0) {
      fine = daysLate * 1;  // RM1/day
      cout << YELLOW << "Late return detected:" << daysLate << " days late."
           << RESET << "\n";
      cout << "Fine amount = RM " << fine << "\n";

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
   record->penalty_amt = fine;
   record->status = 1;
   saveBooks(books);
   saveBorrowRecords(borrow_records);

   cout << GREEN << "Book returned successfully." << RESET << "\n";

   cout << "\nPress Enter to return...";
   cin.ignore();
   cin.get();
}
