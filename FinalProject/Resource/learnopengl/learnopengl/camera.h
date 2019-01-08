#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 1.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;



// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// when collision
	bool backFront;
	bool rightLeft;
	bool cameraCollision;
	bool exitCollision;


	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
		// when collision
		backFront = false;
		rightLeft = false;
		cameraCollision = false;
		exitCollision = false;
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
		// when collision
		backFront = false;
		rightLeft = false;
		cameraCollision = false;
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		// in order to handle 2D movement
		// Right is independent to y axis
		glm::vec3 fakeFront(Front.x, 0.0, Front.z);
		glm::vec3 fakeRight(Right.x, 0.0, Right.z);
		float velocity = MovementSpeed * deltaTime;
		float bounceFactor = -10.0;
		
		// �̫�̥~�h�]�O�_����I�F
		if (exitCollision) {
			//printf("Congrats!\n");
		} else {
			if (direction == FORWARD) {
				if (cameraCollision) {
					// bounce back
					//if (backFront) {
						fakeFront.z = bounceFactor * fakeFront.z;
					//}
					//else if (rightLeft) {
						fakeFront.x = bounceFactor * fakeFront.x;
					//}
				}
				Position += fakeFront * velocity;
			}

			if (direction == BACKWARD) {
				if (cameraCollision) {
					//if (backFront) {
						fakeFront.z = bounceFactor * fakeFront.z;
					//}
					//else if (rightLeft) {
						fakeFront.x = bounceFactor * fakeFront.x;
					//}
				}
				Position -= fakeFront * velocity;
			}

			if (direction == RIGHT) {
				if (cameraCollision) {
					//if (backFront) {
						fakeRight.z = bounceFactor * fakeRight.z;
					//}
					//else if (rightLeft) {
						fakeRight.x = bounceFactor * fakeRight.x;
					//}
				}
				Position += fakeRight * velocity;
			}

			if (direction == LEFT) {
				if (cameraCollision) {
					//if (backFront) {
						fakeRight.z = bounceFactor * fakeRight.z;
					//}
					//else if (rightLeft) {
						fakeRight.x = bounceFactor * fakeRight.x;
					//}
				}
				Position -= fakeRight * velocity;
			}

			if (Position.x > 5)
				Position.x = 4.999999;
			else if (Position.x < -5)
				Position.x = -4.99999;
			if (Position.z > 5)
				Position.z = 4.999999;
			else if (Position.z < -5)
				Position.z = -4.999999;
			Position.y = -4.5;
		}
		
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
#endif