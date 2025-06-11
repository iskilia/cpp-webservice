# C++20 Web Service Boilerplate for AWS

This repository contains boilerplate code for a C++ web service using the httplib, compatible with C++20, and designed for deployment on AWS using Docker.

## Features

* **C++20 Compliance:** Utilizes C++20 features like `std::format`.
* Uses <httplib.h> to start a basic service
* **Dockerized:** Ready for containerization and deployment to AWS.

## Prerequisites

* A C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 19.28+)
* CMake 3.10+
* Docker
* AWS Account and AWS CLI configured

## Local Development

1.  **Clone the repository:**
    ```bash
    git clone <your-repo-url>
    cd cpp-webservice
    ```

2.  **Build and run the Docker image locally:**
    ```bash
    docker-compose up
    ```

3.  **Access the endpoints:**
    * Health Check: `http://localhost:8080/health`
    * Info Page: `http://localhost:8080/info`

## AWS Deployment (High-Level Steps)

This section outlines the general steps for deploying this service to AWS. Specific details like VPC, security groups, and IAM roles are crucial and should be configured securely based on your AWS environment.

1.  **Build the Docker Image:**
    ```bash
    docker build -t cpp-webservice .
    ```

2.  **Create an ECR Repository:**
    ```bash
    aws ecr create-repository --repository-name cpp-aws-web-service
    ```

3.  **Tag and Push the Docker Image to ECR:**
    Follow the `aws ecr get-login-password` command output for login, then tag and push:
    ```bash
    aws ecr get-login-password --region <your-aws-region> | docker login --username AWS --password-stdin <your-aws-account-id>.dkr.ecr.<your-aws-region>.amazonaws.com
    docker tag cpp-webservice:latest <your-aws-account-id>.dkr.ecr.<your-aws-region>[.amazonaws.com/cpp-aws-web-service:latest](https://.amazonaws.com/cpp-aws-web-service:latest)
    docker push <your-aws-account-id>.dkr.ecr.<your-aws-region>[.amazonaws.com/cpp-aws-web-service:latest](https://.amazonaws.com/cpp-aws-web-service:latest)
    ```

4.  **Create an ECS Cluster (if you don't have one):**
    You can use the AWS Console or AWS CLI to create an ECS cluster (e.g., Fargate or EC2 launch type). Fargate is often simpler for serverless container deployment.

5.  **Create an ECS Task Definition:**
    Define your container, image, port mappings (8080), CPU, and memory requirements. Link it to your ECR image.

6.  **Create an ECS Service:**
    Configure the service to run and maintain the desired number of tasks (instances) of your web service within the ECS cluster. Attach an Application Load Balancer (ALB).

7.  **Configure Application Load Balancer (ALB):**
    * Create a Target Group that points to your ECS service on port 8080.
    * Create a Listener on port 80 (HTTP) or 443 (HTTPS) that forwards traffic to your Target Group.
    * Configure security groups to allow inbound traffic to the ALB and from the ALB to your ECS tasks on port 8080.

8.  **Monitor:**
    Use AWS CloudWatch to monitor your service logs and metrics.

## Important Security Considerations

* **IAM Roles:** Use appropriate IAM roles for your ECS tasks with the principle of least privilege.
* **Security Groups:** Strictly control inbound and outbound traffic. Only allow necessary ports.
* **HTTPS:** For production, always use HTTPS. This involves configuring SSL/TLS certificates on your Application Load Balancer.
* **Logging:** Implement robust logging within your C++ application and ensure logs are sent to CloudWatch Logs or another centralized logging solution.
* **Error Handling:** Implement comprehensive error handling and graceful shutdowns in your C++ code.

## Further Enhancements

* **Configuration Management:** Use environment variables or AWS Systems Manager Parameter Store for dynamic configuration (e.g., database connection strings, API keys).
* **Database Integration:** Integrate with AWS databases like DynamoDB, RDS, or Aurora using the AWS SDK for C++.
* **Unit and Integration Tests:** Add comprehensive tests for your endpoints and business logic.
* **CI/CD Pipeline:** Automate the build, test, and deployment process using AWS CodePipeline, GitHub Actions, or Jenkins.
* **Observability:** Integrate with monitoring tools like Prometheus, Grafana, or AWS X-Ray for better tracing and metrics.