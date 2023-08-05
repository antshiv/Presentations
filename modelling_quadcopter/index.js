const RotationDirection = {
	CLOCKWISE: 1,
	COUNTERCLOCKWISE: -1,
};

const LinearMovement = {
	UP: 1,
	DOWN: -1,
	UPDOWN: 0,
};

const HorizontalMovement = {
	LEFT: 1,
	RIGHT: -1,
};

console.log('script loaded');
const quadcopter = new Image();
quadcopter.src = "assets/quadcopter/2048w/quadcopter.png";

const quadcopter_front = new Image();
quadcopter_front.src = "assets/quadcopter/2048w/quadcopter_front.png";

const propeller = new Image();
propeller.src = "assets/propellers/2048w/propeller.png";

function populateCanvas(canvas_id, image, RotationDirectionLocal = null, LinearMovementLocal = null, HorizontalMovementLocal = null) {
	var canvas = document.getElementById(canvas_id);
	if (!canvas) {
		console.log('canvas not found');
		return;
	}
	var context = canvas.getContext("2d");
	if (!context) {
		console.log('context not found');
		return;
	}
	
	image.onload = function (event) {
		const centerX = canvas.width / 2;
		const centerY = canvas.height / 2;

		// Initial position of the image at the center of the canvas
		let x = (canvas.width - image.width) / 2;
		let y = (canvas.height - image.height) / 2;

		// Initial rotation angle (in degrees)
		let rotationAngle = -1;

		// Initial movement direction
		let direction = 1; // 1 for down, -1 for up

		var aspectRatio = image.width / image.height;
		context.drawImage(image, 0, 0, canvas.width, canvas.width / aspectRatio);

		// Function to draw the image at the given angle
		function drawRotatedImage() {
			context.clearRect(0, 0, canvas.width, canvas.height);

			// Save the current context state
			context.save();

			// Set the transform origin to the center of the image
			context.translate(centerX, centerY);
			context.rotate((rotationAngle * Math.PI) / 179); // Convert degrees to radians

			// Draw the scaled image with the top-left corner at the center
			const scaleFactor = 0.2; // Scale factor to half the size
			const scaledWidth = image.width * scaleFactor;
			const scaledHeight = image.height * scaleFactor;

			// Draw the image with the top-left corner at the center
			context.drawImage(image, -scaledWidth / 2, -scaledHeight / 2, scaledWidth, scaledHeight);
			
			// Restore the original context state
			context.restore();
		};

		// Function to draw the image at the current position
		function drawImage(scaledWidth, scaledHeight, yLocal) {
			context.clearRect(0, 0, canvas.width, canvas.height);
			
			// Save the current context state
			context.save();
			context.translate(centerX, centerY);
			
			// Set the transform origin to the center of the image
			// range is -canvan.height /2 to scaledHeight / 2
			context.drawImage(image, -scaledWidth/2, yLocal , scaledWidth, scaledHeight);
			context.restore();
		}

		function updatePosition() {
			if (LinearMovementLocal === null) {
				return;
			}
			const scaleFactor = 0.2; // Scale factor to half the size
			const scaledWidth = image.width * scaleFactor;
			const scaledHeight = image.height * scaleFactor;
			
			// Change the image's position based on the direction
			y += 2 * direction; // Change the movement speed here (2 pixels per frame in this example)
			
			// Check if the image has reached the top or bottom of the canvas
			if (y <= -canvas.height/2 || y  >= scaledHeight / 2) {
				direction *= -1; // Reverse the direction when reaching the canvas boundaries
			}

			drawImage(scaledWidth ,scaledHeight, y); // Redraw the image with the updated position
			requestAnimationFrame(updatePosition);

		}

		// Function to continuously rotate the image
		function animate() {
			if (RotationDirectionLocal === null) {
				return;
			}
			rotationAngle -= RotationDirectionLocal; // Change the rotation speed here (0 degree per frame in this example)
			drawRotatedImage();

			requestAnimationFrame(animate);
		};

		// Start the animation
		animate();
		updatePosition();
	};

	image.onerror = function (event) {
		console.log('image.onerror: Most likely path is not correct');
	};
};


