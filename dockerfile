# Get latest GCC Image as a base
FROM gcc:latest

# Install cJSON library
RUN apt-get update &&\
    apt-get install libcjson-dev libcjson1 -y
    
# Set working directory    
WORKDIR /app

# Copy whole project from current directory 
COPY . .

# Tell docker we will need to expose a port
EXPOSE 12345

# Compile the server
RUN make clean && make

# Run the program
CMD ["./build/server"]