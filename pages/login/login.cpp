#include <sqlite3.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Menu的header
// 用来declare有这个function
#include "../book/add_book.h"

using namespace std;

struct User {
   int id;
   string username;
   string password;
   int attempts;
   bool locked;
   string type;
   string created_at;
};

void getAdminsListing(sqlite3* db) {
   string sql =
       "SELECT id, username, type, locked, attempts, created_at FROM users "
       "WHERE type IN ('admin', 'super_admin')";
   sqlite3_stmt* stmt;

   if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
      cout << "❌ Failed to prepare admin listing query.\n";
      return;
   }

   cout << "\n===== Admin Listing =====\n\n";

   // Table Header
   cout << left << setw(5) << "ID" << setw(15) << "Username" << setw(15)
        << "Role" << setw(10) << "Locked" << setw(10) << "Attempts" << setw(20)
        << "Created At" << endl;

   cout << string(75, '-') << endl;

   // Table Rows
   while (sqlite3_step(stmt) == SQLITE_ROW) {
      int id = sqlite3_column_int(stmt, 0);
      string username = (const char*)sqlite3_column_text(stmt, 1);
      string type = (const char*)sqlite3_column_text(stmt, 2);
      int locked = sqlite3_column_int(stmt, 3);
      int attempts = sqlite3_column_int(stmt, 4);
      string created_at = (const char*)sqlite3_column_text(stmt, 5);

      cout << left << setw(5) << id << setw(15) << username << setw(15) << type
           << setw(10) << (locked ? "YES" : "NO") << setw(10) << attempts
           << setw(20) << created_at << endl;
   }

   sqlite3_finalize(stmt);

   cout << "\nPress Enter to return to menu...";
   cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
   cin.get();
}

bool getUser(sqlite3* db, const string& username, User& user) {
   string sql =
       "SELECT id, username, password, attempts, locked, type, created_at FROM "
       "users WHERE username = ?";
   sqlite3_stmt* stmt;

   sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
   sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

   if (sqlite3_step(stmt) == SQLITE_ROW) {
      user.id = sqlite3_column_int(stmt, 0);
      user.username = (const char*)sqlite3_column_text(stmt, 1);
      user.password = (const char*)sqlite3_column_text(stmt, 2);
      user.attempts = sqlite3_column_int(stmt, 3);
      user.locked = sqlite3_column_int(stmt, 4);
      user.type = (const char*)sqlite3_column_text(stmt, 5);
      user.created_at = (const char*)sqlite3_column_text(stmt, 6);
      sqlite3_finalize(stmt);
      return true;
   }

   sqlite3_finalize(stmt);
   return false;
}

void addAdmin(sqlite3* db) {
   string username, password, type;
   int roleChoice;

   cout << "\n=== Add New Admin ===\n";
   cout << "Enter new admin username: ";
   cin >> username;
   cout << "Enter password for new admin: ";
   cin >> password;

   cout << "\nSelect User Role:\n";
   cout << "1. super_admin\n";
   cout << "2. admin\n";
   cout << "Choose: ";
   cin >> roleChoice;

   if (roleChoice == 1)
      type = "super_admin";
   else if (roleChoice == 2)
      type = "admin";
   else {
      cout << "❌ Invalid choice.\n";
      return;
   }

   string sql =
       "INSERT INTO users (username, password, attempts, locked, type, "
       "created_at) VALUES (?, ?, 0, 0, ?, datetime('now'))";
   sqlite3_stmt* stmt;

   if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
      cout << "❌ Failed to prepare insert query.\n";
      return;
   }

   // table columns
   sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_TRANSIENT);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
      cout << "✅ New Admin Created Successfully!\n";
   } else {
      cout << "❌ Error: This username already exists.\n";
   }

   sqlite3_finalize(stmt);

   cout << "\nPress Enter to return to menu...";
   cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
   cin.get();
}

void updateUser(sqlite3* db, const User& user) {
   string sql =
       "UPDATE users SET password=?, attempts=?, locked=?, type=? WHERE id=?";
   sqlite3_stmt* stmt;

   sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
   sqlite3_bind_text(stmt, 1, user.password.c_str(), -1, SQLITE_STATIC);
   sqlite3_bind_int(stmt, 2, user.attempts);
   sqlite3_bind_int(stmt, 3, user.locked);
   sqlite3_bind_text(stmt, 4, user.type.c_str(), -1, SQLITE_STATIC);
   sqlite3_bind_int(stmt, 5, user.id);

   sqlite3_step(stmt);
   sqlite3_finalize(stmt);
}

void resetUserPassword(sqlite3* db) {
   string targetUser;
   cout << "Enter username to reset password: ";
   cin >> targetUser;

   User x;
   if (!getUser(db, targetUser, x)) {
      cout << "❌ User not found.\n";
      return;
   }

   cout << "Enter new password: ";
   cin >> x.password;
   x.attempts = 0;
   x.locked = 0;

   updateUser(db, x);
   cout << "✅ Password reset complete.\n";
}

void changePassword(User& currentUser, sqlite3* db) {
   string oldPass, newPass;

   cout << "\n=== Change Your Password ===\n";

   cout << "Enter your current password: ";
   cin >> oldPass;

   if (oldPass != currentUser.password) {
      cout << "❌ Wrong password. Cannot change.\n";
      return;
   }

   cout << "Enter new password: ";
   cin >> newPass;

   currentUser.password = newPass;
   currentUser.attempts = 0;
   currentUser.locked = 0;

   string sql = "UPDATE users SET password=?, attempts=0, locked=0 WHERE id=?";
   sqlite3_stmt* stmt;

   sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
   sqlite3_bind_text(stmt, 1, newPass.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_int(stmt, 2, currentUser.id);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
      cout << "✅ Password updated successfully.\n";
   } else {
      cout << "❌ Failed to update password.\n";
   }

   sqlite3_finalize(stmt);

   cout << "\nPress Enter to return to menu...";
   cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
   cin.get();
}

void adminMenu(User& currentUser, sqlite3* db) {
   while (true) {
      cout << "\n--- Admin Menu ---\n";

      vector<pair<int, string>> menu;
      int optionNumber = 1;

      // super_admin 专用
      if (currentUser.type == "super_admin") {
         menu.push_back({optionNumber++, "Add Admin"});
         menu.push_back({optionNumber++, "Reset User Password"});
         menu.push_back({optionNumber++, "Admin Listing"});
      }

      // admin && super_admin 共享
      menu.push_back({optionNumber++, "Add Book"});
      menu.push_back({optionNumber++, "Add Borrower"});
      menu.push_back({optionNumber++, "Borrow Book"});
      menu.push_back({optionNumber++, "Return Book"});
      menu.push_back({optionNumber++, "Change Password"});
      menu.push_back({optionNumber++, "Quit"});

      // show menu
      for (auto& m : menu) {
         cout << m.first << ". " << m.second << endl;
      }

      cout << "Choose: ";
      int choice;
      cin >> choice;

      // 执行对应功能
      int index = choice - 1;
      if (index < 0 || index >= menu.size()) {
         cout << "Invalid option.\n";
         continue;
      }

      string selected = menu[index].second;

      if (selected == "Add Admin") {
         addAdmin(db);
      } else if (selected == "Reset User Password") {
         resetUserPassword(db);
      } else if (selected == "Admin Listing") {
         getAdminsListing(db);
      } else if (selected == "Add Book") {
         cout << "📚 Add Book function here\n";
         addBook(db);
      } else if (selected == "Add Borrower") {
         cout << "👤 Add Borrower function here\n";
      } else if (selected == "Quit") {
         return;
      } else if (selected == "Borrow Book") {
         cout << "📚 Borrow Book function here\n";
      } else if (selected == "Return Book") {
         cout << "📚 Return Book function here\n";
      } else if (selected == "Change Password") {
         changePassword(currentUser, db);
      }
   }
}

int main() {
   while (true) {
      sqlite3* db;
      sqlite3_open("database/users.db", &db);

      string inputUser, inputPass;
      cout << "Enter Username: ";
      cin >> inputUser;
      cout << "Enter Password: ";
      cin >> inputPass;

      User u;
      if (!getUser(db, inputUser, u)) {
         cout << "User not found.\n";
         sqlite3_close(db);
         continue;  // 回到最外层 → 重新登录
      }

      if (u.locked) {
         cout << "Account is locked.\n";
         sqlite3_close(db);
         continue;
      }

      if (u.password == inputPass) {
         u.attempts = 0;
         u.locked = 0;
         updateUser(db, u);

         cout << "Login Successful. Welcome, " << u.username << "!\n";
         if (u.type == "super_admin" || u.type == "admin") {
            adminMenu(u, db);
         } else {
            cout << "Borrower user logged in. Returning to login screen...\n";
         }
      } else {
         cout << "Incorrect password.\n";
         u.attempts++;
         if (u.attempts >= 3) u.locked = 1;
         updateUser(db, u);
      }

      sqlite3_close(db);
   }
}
