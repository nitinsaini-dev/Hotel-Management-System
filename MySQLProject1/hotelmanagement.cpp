#include <iostream>//basic c++
#include <string> // char string 
#include <limits> 
#include <iomanip> // for raw
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>


using namespace std;

// ===== MySQL Global Objects =====
sql::mysql::MySQL_Driver* driver;
sql::Connection* con;
sql::Statement* stmt;
sql::ResultSet* res = nullptr;

// ===== Database Connection =====
void connectDB() {
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "root", "nitinsaini@7467");
    con->setSchema("hotel_db");
    stmt = con->createStatement();
    stmt->execute("USE hotel_db");
}

// ===== Hotel Class =====
class Hotel {
private:
    string customerName;
    string customerAddress;
    int roomNo = 0;
    int days = 0;
    int roomType = 0;
    double roomRent = 0.0;
    double foodBill = 0.0;
    double totle = 0.0;

public:
    // ===== CustomerDetails ==== 
    void enterCustomerDetails() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Enter Customer Name: ";
        getline(cin, customerName);
        cout << "Enter Address: ";
        getline(cin, customerAddress);
        cout << "Enter Room Number: ";
        cin >> roomNo;
        cout << "<---Room Type-->\n 1-Normal=Rs.1000\n 2-AC=Rs.2000\n 3-Luxury=Rs.3000\n";
        cout << "Enter Room_Type::";
        cin >> roomType;
        cout << "Days Stay: ";
        cin >> days;
        cout << endl;
        string checkRoom =
            "SELECT * FROM hotel WHERE room_no=" + to_string(roomNo);
        res = stmt->executeQuery(checkRoom);

        if (!res->next()) 
        { 
            int totalAmount = roomRent + foodBill;
            // ROOM DOES NOT EXIST ->INSERT
            string insertquery =
                "INSERT INTO hotel (name, address, room_no, days, room_rent, food_bill, total, status) VALUES ('" +
                customerName + "','" + 
                customerAddress + "'," +
                to_string(roomNo) + "," + 
                to_string(days) + "," + 
                to_string(roomRent) + "," + 
                to_string(foodBill) + "," + 
                to_string(totalAmount) + ",'BOOKED')";
            cout << insertquery << endl;
            stmt->executeUpdate(insertquery);

        }
        else {
            //ROOM EXISTS -> CHECK STATUS
            if (res->getString("status") == "BOOKED") {
                cout << "Room already booked!\n";
                return;
            }
            else {
                string query =
                    "UPDATE hotel SET "
                    "name='" + customerName + "', "
                    "address='" + customerAddress + "', "
                    "days=" + to_string(days) + ", "
                    "status='BOOKED' "
                    "WHERE room_no=" + to_string(roomNo) + " AND status ='FREE'";
                int rows = stmt->executeUpdate(query);

            }
        }
    }
    void calculateRoomRent() {
        if      (roomType == 1) roomRent = days * 1000;
        else if (roomType == 2) roomRent = days * 2000;
        else if (roomType == 3) roomRent = days * 3000;

        cout << "Room Rent = Rs. " << roomRent << endl;
        string query =
            "UPDATE hotel SET room_rent=" + to_string(roomRent) + 
            ", total=" + to_string(roomRent) + " + food_bill " 
            " WHERE room_no = " + to_string(roomNo) +
            " AND status='BOOKED'";
        stmt->execute(query);
    }
    void foodMenu() {
        int ch, qty;
        int currentfood = 0;
        while (true) {
            cout << "\n1.Tea 20\n2.Coffee 30\n3.Lunch 150\n4.Dinner 200\n5.Exit\n";
            cout << "Choice: ";
            cin >> ch;
            if (ch == 5) break;
            cout << "Quantity: ";
            cin >> qty;
            if      (ch == 1) currentfood += qty * 20;
            else if (ch == 2) currentfood += qty * 30;
            else if (ch == 3) currentfood += qty * 150;
            else if (ch == 4) currentfood += qty * 200;
            else {
                cout << "Invalid choice!\n";
                continue;
            }
            string query =
                " UPDATE hotel SET "
                "food_bill = food_bill + " + to_string(currentfood) + ", "
                "total = room_rent + food_bill + " + to_string(currentfood) +
                " WHERE room_no=" + to_string(roomNo);
            stmt->execute(query);
            cout << "Food Bill = Rs." << currentfood << endl;
        }
  
    }
    void showBill(){
        string query =
            "SELECT name, room_no, room_rent, food_bill, total FROM hotel WHERE room_no=" +
            to_string(roomNo);
        res = stmt->executeQuery(query);
        if (res->next())
        {
            cout << "\n===== FINAL BILL =====\n";
            cout << "Name      :    " << res->getString("name") << endl;
            cout << "Room No   :    " << res->getInt("room_no") << endl;
            cout << "Room Rent : Rs." << res->getInt("room_rent") << endl;
            cout << "Food Bill : Rs." << res->getInt("food_bill") << endl;
            cout << "TOTAL     : Rs." << res->getInt("total") << endl;
            cout << "1.EXIT";
            int ch;
            cin >> ch;
            switch (ch) {
            case 1:
                break;
            }
        }
    } 
    void checkoutCustomer() {
        int roomNo;
        cout << "Enter Room Number for Checkout: ";
        cin >> roomNo;

        sql::PreparedStatement* pstmt = nullptr;
        sql::ResultSet* rs = nullptr;

        // 1. Check room occupied or not
        pstmt = con->prepareStatement(
            "SELECT name, address, room_rent, food_bill  FROM hotel WHERE room_no=? AND status='BOOKED'"
        );
        pstmt->setInt(1, roomNo);
        rs = pstmt->executeQuery();

        if (!rs->next()) {
            cout << "Room not occupied!\n";
            delete rs;
            delete pstmt;
            return;
        }

        string name = rs->getString("name");
        string address = rs->getString("address");
        int roomRent = rs->getInt("room_rent");
        int foodBill = rs->getInt("food_bill");
        int total = roomRent + foodBill;

        // 2. Show final bill
        cout << "\n===== FINAL BILL =====\n";
        cout << "Name      : " << name << endl;
        cout << "Address   : " << address << endl;
        cout << "Room No   : " << roomNo << endl;
        cout << "Room Rent : " << roomRent << endl;
        cout << "Food Bill : " << foodBill << endl;
        cout << "Total     : " << total << endl;

        // 3. Insert into checkout history
        pstmt = con->prepareStatement(
            "INSERT INTO checkout_history(room_no, customer_name, address, room_rent, food_bill, total_bill ) VALUES (?,?,?,?,?,?)"
        );
        pstmt->setInt(1, roomNo);
        pstmt->setString(2, name);
        pstmt->setString(3, address);
        pstmt->setInt(4, roomRent);
        pstmt->setInt(5, foodBill);
        pstmt->setInt(6, total);
        pstmt->executeUpdate();

        // 4. Free the room
        pstmt = con->prepareStatement(
            "UPDATE hotel SET status='FREE', name=NULL, address=NULL, days=0, "
            "room_rent=0, food_bill=0, total=0 WHERE room_no=?"
        );
        pstmt->setInt(1, roomNo);
        pstmt->executeUpdate();
        cout << endl;
        cout << "=====Checkout successful & history saved!=====\n";

        delete rs;
        delete pstmt;
    }
    void checkoutHistory()
    {
        sql::Statement* stmt;
        sql::ResultSet* rs;
        stmt = con->createStatement();
        rs = stmt->executeQuery(
            "SELECT room_no, customer_name, address, room_rent, food_bill, total_bill,  checkout_date"
            " FROM checkout_history "
        );
            cout << "\n<================== CHECKOUT HISTORY ==================>\n";
            cout << endl;
            // table Header 
            cout << left
                << setw(8)  << "Room"
                << setw(15) << "Name"
                << setw(20) << "Address"
                << setw(8)  << "Rent"
                << setw(8)  << "Food"
                << setw(8)  << "TOtel"
                << setw(15) << "Date & Time"<<endl;
            cout << string(100, '-') << endl;
        bool found = false;
        while (rs->next()) {
            found = true;
            cout << left
                << setw(8) << rs->getInt("room_no")
                << setw(15) << rs->getString("customer_name")
                << setw(20) << rs->getString("address")
                << setw(8) << rs->getInt("room_rent")
                << setw(8) << rs->getInt("food_bill")
                << setw(8) << rs->getInt("total_bill")
                << setw(15) << rs->getString("checkout_date")
                << endl;
        }
        if (!found) {
            cout << " No Checkout history found!\n";
        }
    }
    void addFoodOrder() {
        int roomNo, foodPrice;
        cout << "1.Enter Room No: ";
        cin >> roomNo;
        cout << "2.Enter Food Amount: ";
        cin >> foodPrice;
        string query =
            "UPDATE hotel SET "
            "food_bill = food_bill + " + to_string(foodPrice) + ", "
            "total = room_rent + food_bill + " + to_string(foodPrice) +
            " WHERE room_no = " + to_string(roomNo) +
            " AND status='BOOKED'";
        stmt->executeQuery(query);
        cout << " Food Order added successfully!\n";
    }
   
};
// ===== Main =====
int main() {
    try {
        connectDB();
        Hotel h;
        int choice;

        while (true) {
            cout << string(100, '-') <<endl;
            cout << "<======= HOTEL MANAGEMENT SYSTEM =======>\n";
            cout << "\n1.New Customer..\n2.Show Room Rent.Rs.\n3.Food Menu\n4.Show Bill..\n5.Checkout Customer\n6.Show Checkout History\n7.Add Food Order\n8.Exit\n";
            cout << string(100, '-') << endl;
            cout << " Enter Choice: ";
            cin >> choice;
            if (choice == 1) h.enterCustomerDetails();
            else if (choice == 2) h.calculateRoomRent();
            else if (choice == 3) h.foodMenu();
            else if (choice == 4) h.showBill();
            else if (choice == 5) h.checkoutCustomer();
            else if (choice == 6) h.checkoutHistory();
            else if (choice == 7) h.addFoodOrder();
            else if (choice == 8) break;
            else cout << "Invalid Choice\n";
        }
        delete stmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cout << "MySQL Error: " << e.what() << endl;
    }
    return 0;
}

