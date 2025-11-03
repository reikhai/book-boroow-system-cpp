#include "../../struct/add_book.h"

#include <iostream>
using namespace std;

void addBook() {
   // 在这里写logic
   cout << "\nPress Enter to return to menu...";
   cout << "\nHello from addBook function!" << endl;
   cin.ignore();
   cin.get();  // Wait for user input before returning
}
