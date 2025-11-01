#ifndef RETURN_BOOK_H
#define RETURN_BOOK_H

#include <vector>
#include <string>
#include <iostream>

// struct （有用到就要加进来)
#include "borrower.h"
#include "book.h"
#include "borrow_record.h"

using namespace std;

void returnBook(vector<Borrower> &borrowers,
                vector<Book> &books,
                vector<BorrowRecord> &records);

#endif
