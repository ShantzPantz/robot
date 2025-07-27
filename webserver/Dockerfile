# Use a slim Python base image for smaller image size
FROM python:3.12-slim

# Set the working directory inside the container
WORKDIR /app

# Copy the requirements file and install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the entire Flask application
COPY /src .

# Expose the port your Flask app will run on (e.g., 5000)
EXPOSE 5000

# Define environment variables (optional but often useful for Flask)
ENV FLASK_APP=main.py
ENV FLASK_RUN_HOST=0.0.0.0

# Command to run the Flask application
CMD ["flask", "run"]