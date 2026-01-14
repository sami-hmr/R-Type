# Use the official Node.js image as the base image
FROM node:20


# Set the working directory inside the container
WORKDIR /app

# Copy package.json and package-lock.json to the working directory
COPY package*.json ./

# Install the application dependencies
RUN npm install

# Copy the application files
COPY . .

EXPOSE 3000

# Command to run the application
CMD ["npm", "run", "start:dev"]
