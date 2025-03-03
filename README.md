# SPL-Assignment4-SQLite-Python (BGU Mart)

## Overview
This project implements a database management system for a supermarket chain called "BGU Mart" using Python and SQLite. The system manages employees, suppliers, products, branches, and activities (sales and deliveries).

## Features
- Employee management system
- Supplier database
- Product inventory tracking
- Branch location management
- Activity logging for sales and deliveries
- Data persistence using SQLite

## Files Structure
- `initiate.py`: Builds the database and inserts initial data from a configuration file
- `action.py`: Manages supermarket activities (sales and deliveries)
- `printdb.py`: Prints database contents and generates reports
- `persistence.py`: Defines data models and database connection
- `dbtools.py`: Provides database utility functions and ORM implementation

## How to Use
1. Initialize the database with sample data:
   ```
   python initiate.py config.txt
   ```

2. Perform activities (sales/deliveries):
   ```
   python action.py action.txt
   ```

3. Print database contents and reports:
   ```
   python printdb.py
   ```

## Requirements
- Python 3.9 or higher
- SQLite3

## Implementation Details
The project follows the DAO (Data Access Objects) and DTO (Data Transfer Objects) design patterns for database access, with a Repository pattern for overall data management. The ORM implementation allows for flexible database operations without direct SQL in most places.

## Academic Notice
This project was developed as part of the System Programming Languages course at Ben-Gurion University.
