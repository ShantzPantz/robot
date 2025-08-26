# Use a slim Python base image for smaller image size
FROM python:3.11-slim

# RUN apt update && apt install -y ffmpeg
RUN apt-get update && apt-get install -y build-essential python3-dev && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /app

# Copy the requirements file and install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the entire application
COPY /src .

# Download pre-trained models once
RUN python -c "import openwakeword.utils; openwakeword.utils.download_models()"

# port to expose
EXPOSE 8000

# Command to run the application
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]