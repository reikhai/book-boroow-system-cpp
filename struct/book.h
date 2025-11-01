#ifndef BOOK_H
#define BOOK_H

#include <string>

struct Book
{
   int id;
   std::string title, author, isbn;
   int copies;
   std::string created_at;
};

#endif
