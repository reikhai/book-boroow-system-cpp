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
   cout << "\n=== Return Book ===\n";
   cout << "Enter borrower name: ";
   getline(cin, borrowerName);
   cout << "Enter book title: ";
   getline(cin, bookTitle);

   // 找 borrower
   int borrowerId = -1;
   for (auto& b : borrowers)
      if (b.name == borrowerName) borrowerId = b.id;

   if (borrowerId == -1) {
      cout << "❌ Borrower not found.\n";
      return;
   }

   // 找 book
   int bookId = -1;
   for (auto& bk : books)
      if (bk.title == bookTitle) bookId = bk.id;

   if (bookId == -1) {
      cout << "❌ Book not found.\n";
      return;
   }

   // 找未归还记录 (status == 0)
   BorrowRecord* record = nullptr;
   for (auto& r : borrow_records)
      if (r.borrower_id == borrowerId && r.book_id == bookId && r.status == 0)
         record = &r;

   if (!record) {
      cout << "⚠️ No active borrow record.\n";
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
      cout << "\n⚠️ Late return detected: " << daysLate << " days late.\n";
      cout << "💰 Fine amount = RM " << fine << "\n";

      string payChoice;
      cout << "Pay now? (yes/no): ";
      cin >> payChoice;

      if (payChoice != "yes") {
         cout << "❌ Return cancelled. Please settle payment first.\n";
         return;
      }

      cout << "✅ Payment received.\n";
   }
   record->penalty_amt = fine;
   record->status = 1;
   saveBooks(books);
   saveBorrowRecords(borrow_records);

   cout << "✅ Book returned successfully.\n";
   cout << "\nPress Enter to return...";
   cin.ignore();
   cin.get();
}
