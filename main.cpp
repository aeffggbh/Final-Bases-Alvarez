#include <iostream>
#include "mysqlx/xdevapi.h"

using namespace std;
using namespace mysqlx;

enum class MenuOption
{
	SHOW_AUTHORS = 1,
	SHOW_BOOKS,
	ADD_USER,
	REGISTER_LOAN,
	SHOW_LOANS,
	EXIT
};

Session Connect();

void ShowMenu();

void ShowTable(Schema& db, std::string tableName, vector<std::string>& columns);

void AddUser(Schema& db);

void RegisterLoan(Schema& db);

void InitializeBooks(Schema& db);

std::string GetLocalDate();

std::string GetDateAfterDays(int days);

int main()
{
	std::string dbName = "bookkeep";
	Session session = Connect();
	Schema db = session.getSchema(dbName);

	InitializeBooks(db);

	MenuOption option;

	vector<std::string> authorColumns = { "id", "name" };
	vector<std::string> bookColumns = { "id", "title", "author_id", "category_id" };
	vector<std::string> loanColumns = { "id", "user_id", "book_id", "loan_date", "return_date" };

	do
	{
		system("cls");
		int input;
		ShowMenu();
		cin >> input;

		option = static_cast<MenuOption>(input);

		switch (option)
		{
		case MenuOption::SHOW_AUTHORS:
			ShowTable(db, "authors", authorColumns);
			break;
		case MenuOption::SHOW_BOOKS:
			ShowTable(db, "books", bookColumns);
			break;
		case MenuOption::ADD_USER:
			AddUser(db);
			break;
		case MenuOption::REGISTER_LOAN:
			RegisterLoan(db);
			break;
		case MenuOption::SHOW_LOANS:
			ShowTable(db, "loans", loanColumns);
			break;
		case MenuOption::EXIT:
			cout << "Exiting the library management system. Goodbye!" << endl;
			break;
		default:
			break;
		}

		system("pause");

	} while (option != MenuOption::EXIT);

}

Session Connect()
{
	std::string user = "root";
	std::string password;
	int port = 33060;
	std::string host = "localhost";

	cout << "Enter password for user '" << user << "': ";
	getline(cin, password);


	try
	{
		Session session(host, port, user, password);
		cout << "Connected to MySQL server successfully!" << endl;
		return session;
	}
	catch (const mysqlx::Error& e)
	{
		cerr << "Error connecting to MySQL server: " << e.what() << endl;
		exit(EXIT_FAILURE);
	}

}

void ShowMenu()
{
	cout << "Welcome to the library management system!" << endl;
	cout << "Please choose an option:" << endl;
	cout << "1. Show authors" << endl;
	cout << "2. Show books" << endl;
	cout << "3. Add user" << endl;
	cout << "4. Register a loan" << endl;
	cout << "5. Show loans" << endl;
	cout << "6. Exit" << endl;
}

void ShowTable(Schema& db, std::string tableName, vector<std::string>& columns)
{
	try
	{
		Table table = db.getTable(tableName);
		RowResult result = table.select(columns).execute();

		cout << tableName << endl;
		for (auto row : result)
		{
			cout << "------------------------" << endl;
			for (int i = 0; i < columns.size(); i++)
			{
				cout << columns[i] << ": " << row[i] << "  " << endl;
			}
		}
		cout << "------------------------" << endl;
		cout << endl;

	}
	catch (const mysqlx::Error& e)
	{
		cout << "Error showing table '" << tableName << "': " << e.what() << endl;
	}
}

void AddUser(Schema& db)
{
	std::string name;

	cout << "Adding a new user" << endl;
	cout << "Enter user name: ";
	cin.ignore();
	getline(cin, name);

	try
	{
		Table users = db.getTable("users");
		users.insert("name").values(name).execute();
		cout << "User '" << name << "' added successfully!" << endl;
	}
	catch (const mysqlx::Error& e)
	{
		cerr << "Error adding user '" << name << e.what() << endl;
	}
}

void RegisterLoan(Schema& db)
{
	int userId;
	int bookId;

	cout << "Registering a new loan" << endl;
	cout << "Enter user ID: ";
	cin >> userId;
	cout << "Enter book ID: ";
	cin >> bookId;

	std::string loanDate = GetLocalDate();
	std::string returnDate = GetDateAfterDays(30);

	try
	{
		Table loans = db.getTable("loans");
		loans.insert("user_id", "book_id", "loan_date", "return_date")
			.values(userId, bookId, loanDate.c_str(), returnDate.c_str())
			.execute();
		cout << "Loan registered successfully!" << endl;
		cout << "Loan Date: " << loanDate << ", Return Date: " << returnDate << endl;
	}
	catch (const mysqlx::Error& e)
	{
		cerr << "Error registering loan for user ID " << userId << " and book ID " << bookId << ": " << e.what() << endl;
	}

}

void InitializeBooks(Schema& db)
{
	try
	{
		Table authors = db.getTable("authors");
		Table categories = db.getTable("categories");
		Table books = db.getTable("books");

		RowResult res = authors.select("COUNT(*)").execute();

		int authorCount = res.fetchOne()[0].get<int>();

		if (authorCount > 0)
			return;

		authors.insert("name")
			.values("George Orwell")
			.values("Gabriel Garcia Marquez")
			.values("J.R.R. Tolkien")
			.values("J.K. Rowling")
			.values("Ray Bradbury")
			.execute();

		categories.insert("name")
			.values("Dystopian")
			.values("Magical Realism")
			.values("Fantasy")
			.execute();

		books.insert("title", "author_id", "category_id")
			.values("1984", 1, 1)
			.values("One Hundred Years of Solitude", 2, 2)
			.values("The Hobbit", 3, 3)
			.values("Harry Potter and the Philosopher's Stone", 4, 3)
			.values("Fahrenheit 451", 5, 1)
			.execute();
	}
	catch (const mysqlx::Error& e)
	{
		cerr << "Error initializing books: " << e.what() << endl;
	}
}

std::string GetLocalDate()
{
	time_t now = time(0);
	tm localTime;
	localtime_s(&localTime, &now);

	char buffer[11];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d", &localTime);
	return std::string(buffer);
}

std::string GetDateAfterDays(int days)
{
	time_t now = time(0);
	now += days * 24 * 60 * 60;

	tm newDate;
	localtime_s(&newDate, &now);

	char buffer[11];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d", &newDate);
	return std::string(buffer);
}