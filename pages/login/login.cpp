#include <sqlite3.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

void adminMenu(User& currentUser, sqlite3* db) {
   while (true) {
      cout << "\n--- Admin Menu ---\n";

      vector<pair<int, string>> menu;
      int optionNumber = 1;

      // super_admin 专用
      if (currentUser.type == "super_admin") {
         menu.push_back({optionNumber++, "Reset User Password"});
         menu.push_back({optionNumber++, "Admin Listing"});
      }

      // admin && super_admin 共享
      if (currentUser.type == "super_admin" || currentUser.type == "admin") {
         menu.push_back({optionNumber++, "Add Book"});
         menu.push_back({optionNumber++, "Add Borrower"});
         menu.push_back({optionNumber++, "Borrow Book"});
         menu.push_back({optionNumber++, "Return Book"});
      }

      // Quit 永远最后
      menu.push_back({optionNumber++, "Quit"});

      // 显示菜单
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

      if (selected == "Reset User Password") {
         resetUserPassword(db);
      } else if (selected == "Admin Listing") {
         cout << "\n--- Admin List ---\n";
         // TODO: print user list
      } else if (selected == "Add Book") {
         cout << "📚 Add Book function here\n";
      } else if (selected == "Add Borrower") {
         cout << "👤 Add Borrower function here\n";
      } else if (selected == "Quit") {
         return;
      }
   }
}

int main() {
   while (true) {  // 整个系统循环

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
