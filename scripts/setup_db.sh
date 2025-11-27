#!/bin/bash

# Setup database for Online Testing Application

DB_FILE="testing_app.db"
SCHEMA_FILE="database/schema.sql"
INIT_DATA_FILE="database/init_data.sql"

echo "Setting up database..."

# Remove old database if exists
if [ -f "$DB_FILE" ]; then
    echo "Removing old database..."
    rm "$DB_FILE"
fi

# Create new database with schema
echo "Creating database schema..."
sqlite3 "$DB_FILE" < "$SCHEMA_FILE"

if [ $? -ne 0 ]; then
    echo "Error creating schema!"
    exit 1
fi

# Insert sample data
echo "Inserting sample data..."
sqlite3 "$DB_FILE" < "$INIT_DATA_FILE"

if [ $? -ne 0 ]; then
    echo "Error inserting data!"
    exit 1
fi

echo "Database setup completed successfully!"
echo "Database file: $DB_FILE"

# Show statistics
echo ""
echo "Database statistics:"
echo "- Users: $(sqlite3 $DB_FILE 'SELECT COUNT(*) FROM Users;')"
echo "- Questions: $(sqlite3 $DB_FILE 'SELECT COUNT(*) FROM Questions;')"
echo "  - Easy: $(sqlite3 $DB_FILE 'SELECT COUNT(*) FROM Questions WHERE difficulty=\"easy\";')"
echo "  - Medium: $(sqlite3 $DB_FILE 'SELECT COUNT(*) FROM Questions WHERE difficulty=\"medium\";')"
echo "  - Hard: $(sqlite3 $DB_FILE 'SELECT COUNT(*) FROM Questions WHERE difficulty=\"hard\";')"

