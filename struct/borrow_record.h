#ifndef BORROW_RECORD_H
#define BORROW_RECORD_H

#include <string>

struct BorrowRecord
{
   int id, borrower_id, book_id, quantity, status;
   std::string borrow_date, return_date, return_at;
   int penalty_amt;
   std::string created_at;
   int created_by;
};

#endif
