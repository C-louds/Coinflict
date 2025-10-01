

# Coinflict

**Coinflict** is a C++ desktop application for managing and analyzing personal finances. It provides an intuitive GUI built with [ImGui](https://github.com/ocornut/imgui), [ImPlot](https://github.com/epezent/implot), and [GLFW](https://www.glfw.org/). The app supports importing transactions (including from PDFs), visualizing analytics, and tracking spending over time.

---

## Table of Contents

* [Features](#features)
* [Installation](#installation)
* [Usage](#usage)
* [Configuration](#configuration)
* [Database Setup](#database-setup)
* [Dependencies](#dependencies)
* [Troubleshooting](#troubleshooting)
* [Contributors](#contributors)
* [License](#license)

---

## Features

* **Dashboard**: Overview of balance, spending trends, and pending transactions.
* **Transactions**: Import, view, and filter all financial transactions.
* **Analytics**: Charts and statistics powered by ImPlot.
* **PDF Import**: Parse financial statements into structured transaction data.
* **Date Picker**: Intuitive time-based filtering of data (Yet to implement).
* Checkout [TODO.md](app/TODO.md) if interested.

---

## Installation

### Prerequisites

* C++17 or newer
* [CMake](https://cmake.org/) ≥ 3.10
* [GLFW](https://www.glfw.org/)
* [ImGui](https://github.com/ocornut/imgui) (included in `external/`)
* [ImPlot](https://github.com/epezent/implot) (included in `external/`)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/coinflict.git
cd coinflict

# Create a build directory
mkdir build && cd build

# Generate build files
cmake ..

# Compile the project
make

# Run the application
./Coinflict
```

> Note: On Windows, you can use `cmake --build . --config Release` and run `Coinflict.exe`.

---

## Usage

1. Launch the application.
2. Use the **Dashboard** to see an overview of your finances.
3. Import transactions via CSV or PDF.
4. Use the **Analytics** tab for charts and spending trends.
5. Click on tabs or buttons to navigate and filter data.

---

## Configuration

* Currently, configuration is minimal.
* Future versions may include a settings file for theme, currency, and default import paths.

---

## Database Setup

Coinflict uses a SQL database to store transactions and analytics. You can set up the database using the provided `schema.sql` file and a `.env` configuration file.

### 1️⃣ Create a `.env` File

Create a file named `.env` in the root of the project with the following variables:

```env
DB_HOST=localhost
DB_NAME=coinflict_db
DB_USER=your_username
DB_PASSWORD=your_password
```

* Replace `your_username` and `your_password` with your database credentials.
* Ensure that the database server is running and accessible.

---

### Customize the Schema

The `schema.sql` file contains the structure of the database, including the `user` column. Update the `user` field in the schema to match your username from `.env`:

```sql
-- Example from schema.sql
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(255) NOT NULL DEFAULT 'your_username',
    email VARCHAR(255),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

* Replace `'your_username'` with the value of `DB_USER` from your `.env` file.

---

### Create the Database

You can create the database and tables using the schema:

```bash
# Using psql (PostgreSQL example)
psql -h $DB_HOST -U $DB_USER -d $DB_NAME -f schema.sql
```

* This will create all tables with your configured username.
* Make sure your `.env` credentials match the database user and permissions.

---

### 4️⃣ Connect Coinflict to the Database

The application reads the `.env` file to connect to your database. Once your `.env` is configured and the schema is applied, Coinflict will automatically connect using those credentials.

---

## Dependencies

* [GLFW](https://www.glfw.org/) – Windowing and input.
* [ImGui](https://github.com/ocornut/imgui) – Immediate mode GUI.
* [ImPlot](https://github.com/epezent/implot) – Plotting and charts.

---

## Troubleshooting

* Ensure all dependencies are available and paths are correctly set.
* If PDF import fails, check that the PDF format matches supported layouts(It won't work rn anyways).
* For compilation issues, verify your C++ compiler supports C++17 or newer.

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---
